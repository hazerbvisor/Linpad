// SPDX-License-Identifier: GPL-3.0-only
#ifndef LINPAD_FILE_BRIDGE_H
#define LINPAD_FILE_BRIDGE_H
#include <stddef.h>
#include <stdint.h>
// Caller owns a trusted descriptor for this app's current fakefs data directory.
// Every subsequent guest-controlled component is opened without following links.
int linpad_display_open_session(int root, const char *identifier);
enum linpad_display_file { LINPAD_FRAME, LINPAD_SERVER_LOG, LINPAD_CLIENT_LOG };
int linpad_display_read(int session, enum linpad_display_file, uint8_t **, size_t *);
// Append one bounded allowlisted event to an existing guest-created regular file.
int linpad_display_event(int session, const char *event);
#endif
