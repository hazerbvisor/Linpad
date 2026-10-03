# SPDX-License-Identifier: GPL-3.0-only
import hashlib
import importlib.util
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest
from unittest import mock
import zipfile

PROJECT = Path(__file__).resolve().parents[2]

def module(name, path):
    spec = importlib.util.spec_from_file_location(name, path)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result

archive = module('archive_prepare', PROJECT/'scripts/prepare-prebuilt.py')
vdso = module('vdso_prepare', PROJECT/'vdso/arm64/prepare-prebuilt.py')

class ArchiveCacheTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.zip = self.root/'fixture.zip'
        self.contents = {'libarchive.xcframework/ios-arm64/libarchive.a': b'!<arch>\nfixture',
                         'libarchive.xcframework/ios-arm64/Headers/archive.h': b'header'}
        with zipfile.ZipFile(self.zip, 'w') as z:
            for name,data in self.contents.items(): z.writestr(name,data)
        self.manifest = {'version':'test','url':'https://example.invalid/fixture.zip',
                         'sha256':archive.digest(self.zip),'size':self.zip.stat().st_size,
                         'files':[{'path':n,'size':len(b),'sha256':hashlib.sha256(b).hexdigest()}
                                  for n,b in self.contents.items()]}
        self.cache = self.root/'cache with spaces'

    def prepare(self):
        return archive.prepare(self.cache, self.manifest, self.zip)

    def test_offline_valid_cache_needs_no_zip_or_network(self):
        destination = self.prepare()
        self.zip.unlink()
        with mock.patch.object(archive.subprocess,'run', side_effect=AssertionError('network used')):
            self.assertEqual(destination, self.prepare())

    def test_corrupt_extracted_member_repaired_from_verified_zip(self):
        destination = self.prepare()
        name = next(iter(self.contents))
        (destination/name).write_bytes(b'corrupt')
        self.prepare()
        self.assertEqual((destination/name).read_bytes(),self.contents[name])

    def test_invalid_zip_does_not_replace_previous_members(self):
        destination = self.prepare()
        names = list(self.contents)
        (destination/names[1]).unlink() # Require preparation instead of using valid cache.
        self.zip.write_bytes(b'truncated')
        with self.assertRaisesRegex(ValueError,'SHA-256 mismatch'): self.prepare()
        self.assertEqual((destination/names[0]).read_bytes(),self.contents[names[0]])

    def test_wrong_member_hash_rejected_before_output(self):
        self.manifest['files'][1]['sha256'] = '0'*64
        with self.assertRaisesRegex(ValueError,'Member SHA-256 mismatch'): self.prepare()
        self.assertFalse((self.cache/'libarchive-test').exists())

    def test_traversal_manifest_rejected(self):
        self.manifest['files'][0]['path'] = '../escape'
        with self.assertRaisesRegex(ValueError,'Unsafe'): self.prepare()
        self.assertFalse((self.root/'escape').exists())

    def test_failed_download_leaves_no_partial_cached_archive(self):
        def fail(command,check):
            Path(command[-1]).write_bytes(b'partial')
            raise subprocess.CalledProcessError(22,command)
        with mock.patch.object(archive.subprocess,'run',side_effect=fail):
            with self.assertRaises(subprocess.CalledProcessError):
                archive.prepare(self.cache,self.manifest)
        self.assertFalse((self.cache/'libarchive-test.zip').exists())
        self.assertEqual(list(self.cache.glob('.download-*')),[])

class VDSOPinTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.root = Path(self.temp.name)
        self.vdso_root = self.root/'vdso/arm64'
        shutil.copytree(PROJECT/'vdso/arm64',self.vdso_root)
        (self.root/'kernel').mkdir()
        shutil.copyfile(PROJECT/'kernel/vdso.h',self.root/'kernel/vdso.h')

    def test_real_pinned_payload_valid(self):
        self.assertEqual(vdso.validate(self.vdso_root),(self.vdso_root/'prebuilt/libvdso.so.elf').read_bytes())

    def test_stale_source_rejected(self):
        with (self.vdso_root/'vdso.c').open('a') as f: f.write('\n// changed\n')
        with self.assertRaisesRegex(ValueError,'Stale prebuilt VDSO input'): vdso.validate(self.vdso_root)

    def test_changed_runtime_allocation_header_rejected(self):
        (self.root/'kernel/vdso.h').write_text('#define VDSO_PAGES 1\n')
        with self.assertRaisesRegex(ValueError,'Stale prebuilt VDSO input'): vdso.validate(self.vdso_root)

    def test_corrupt_payload_rejected(self):
        (self.vdso_root/'prebuilt/libvdso.so.elf').write_bytes(b'')
        with self.assertRaisesRegex(ValueError,'SHA-256 mismatch'): vdso.validate(self.vdso_root)

    def test_wrong_architecture_rejected_even_with_refreshed_hash(self):
        p=self.vdso_root/'prebuilt/libvdso.so.elf';data=bytearray(p.read_bytes())
        data[18:20]=b'\x3e\0';p.write_bytes(data)
        m=self.vdso_root/'prebuilt/manifest.json';meta=json.loads(m.read_text())
        meta['sha256']=archive.digest(p);m.write_text(json.dumps(meta))
        with self.assertRaisesRegex(ValueError,'AArch64'): vdso.validate(self.vdso_root)

if __name__=='__main__': unittest.main()
