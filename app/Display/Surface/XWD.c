// SPDX-License-Identifier: GPL-3.0-only
#include "XWD.h"
#include <stdlib.h>
#include <string.h>
static uint32_t word(const uint8_t *p) {
    return (uint32_t)p[0]<<24 | (uint32_t)p[1]<<16 | (uint32_t)p[2]<<8 | p[3];
}
const char *linpad_xwd_decode(const uint8_t *data, size_t size, struct linpad_surface *out) {
    if (out == NULL) return "Missing output surface";
    *out = (struct linpad_surface){0};
    if (data == NULL || size < 100 || size > LINPAD_MAX_FRAME) return "Invalid XWD size";
    uint32_t h[25];
    for (unsigned i=0; i<25; i++) h[i]=word(data+4*i);
    if (h[0]<101 || h[0]>4096 || h[0]>size || data[h[0]-1]!=0 || h[1]!=7)
        return "Invalid XWD header";
    if (h[2]!=2 || h[3]!=24 || h[6]!=0 || h[7]>1 || h[8]!=32 || h[9]>1 ||
        h[10]!=32 || (h[11]!=24 && h[11]!=32) || h[13]!=4 ||
        h[14]!=0xff0000 || h[15]!=0x00ff00 || h[16]!=0x0000ff || h[17]!=8 || h[18]>4096)
        return "Unsupported XWD pixel format (requires RGB888 TrueColor)";
    uint32_t w=h[4], height=h[5], stride=h[12], pixel=h[11]/8;
    if (!w || !height || w>LINPAD_MAX_WIDTH || height>LINPAD_MAX_HEIGHT ||
        stride<w*pixel || stride>LINPAD_MAX_WIDTH*4+256 || h[19]>4096)
        return "Invalid XWD geometry";
    uint64_t offset=(uint64_t)h[0]+(uint64_t)h[19]*12;
    uint64_t length=(uint64_t)stride*height;
    if (offset>size || length>size-offset || offset+length!=size)
        return "Truncated or inconsistent XWD pixels";
    size_t rgba_size=(size_t)w*height*4;
    uint8_t *rgba=malloc(rgba_size);
    if (!rgba) return "Surface allocation failed";
    for (uint32_t y=0; y<height; y++) {
        const uint8_t *row=data+(size_t)offset+(size_t)y*stride;
        for (uint32_t x=0; x<w; x++) {
            const uint8_t *p=row+(size_t)x*pixel;
            uint8_t *dst=rgba+((size_t)y*w+x)*4;
            if (h[7]==0) { dst[0]=p[2]; dst[1]=p[1]; dst[2]=p[0]; }
            else { dst[0]=p[pixel-3]; dst[1]=p[pixel-2]; dst[2]=p[pixel-1]; }
            dst[3]=255;
        }
    }
    out->width=w; out->height=height; out->rgba=rgba;
    return NULL;
}
void linpad_surface_free(struct linpad_surface *s) {
    if (s) { free(s->rgba); memset(s,0,sizeof(*s)); }
}
