// SPDX-License-Identifier: GPL-3.0-only
#ifndef LINPAD_XWD_H
#define LINPAD_XWD_H
#include <stddef.h>
#include <stdint.h>
#define LINPAD_MAX_WIDTH 1024u
#define LINPAD_MAX_HEIGHT 768u
#define LINPAD_MAX_FRAME (4u * 1024u * 1024u)
struct linpad_surface { uint32_t width, height; uint8_t *rgba; };
// XWD header words are big-endian; pixel byte order is declared separately.
// Only bounded 24-depth TrueColor ZPixmap with RGB888 masks is accepted.
const char *linpad_xwd_decode(const uint8_t *, size_t, struct linpad_surface *);
void linpad_surface_free(struct linpad_surface *);
#endif
