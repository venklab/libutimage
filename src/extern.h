#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


typedef uint8_t ubyte;

extern int distance_by_dilate(ubyte *mask1, ubyte *mask2, int w, int h);

extern void color_slice(ubyte *im, ubyte *mask, int w, int h,
                        ubyte r, ubyte g, ubyte b, int r0);

extern void resize_mask(bool *mask, int mw, int mh,
                        bool *outim, int w, int h);

extern void highlight_mask_border(ubyte *mask, int w, int h,
                                  uint32_t *out_area,
                                  uint32_t *out_border_len);

extern void gamma_correct(ubyte *im, int w, int h, int ch,
                          double gamma, double c);

extern void contrast_stretch(ubyte *im, int w, int h, ubyte low, ubyte high);

extern void fluor_filter(ubyte *im, int w, int h, int amp_b, int color_b,
                         int amp_g, int color_g, int amp_r, int color_r);
