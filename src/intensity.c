#include "extern.h"
#include <math.h>
#include <stdlib.h>
#define BLU 0xFF0000
#define GRN 0x00FF00
#define RED 0x0000FF
#define MAX(a,b) (((a)>(b))?(a):(b))

/* im is a RGB image, apply gamma in-place
 * s = cr^gamma */
void gamma_correct(ubyte *im, int w, int h, int ch, double gamma, double c)
{
    int64_t i;
    int64_t len = w * h * ch;
    double tmp;
    ubyte lut[256];

    for (i = 0; i < 256; i++) {
        tmp = c * round(255.0 * pow((double)i / 255.0, gamma));
        lut[i] = tmp > 255 ? 255 : (ubyte)tmp;
    }

    for (i = 0; i < len; i++)
        im[i] = lut[im[i]];
}


/* im is a RGB image, stretch image linearly between low and high in-place */
void contrast_stretch(ubyte *im, int w, int h, ubyte low, ubyte high)
{
    int64_t i, tmp;
    int64_t len = w * h * 3;
    double f = 255.0 / high;
    ubyte lut[256];

    for (i = 0; i < 256; i++) {
        if (i >= high) {
            lut[i] = 254;
            continue;
        } else if (i < low) {
            lut[i] = 0;
            continue;
        }

        tmp = round(i * f);
        lut[i] = (tmp >= 254) ? 254 : (ubyte)tmp;
    }

    for (i = 0; i < len; i++)
        im[i] = lut[im[i]];
}


/* im is a RGB fluorescence image */
void apply_filter_one_channel(ubyte *im, int w, int h, ubyte *buf, int amp,
                              int color, int ch)
{
    double f, c;
    ubyte lut[256];
    int b, g, r, v;
    int64_t offset, i, len = w * h;

    r = color >> 16;
    g = (color & 0x00FF00) >> 8;
    b = color & 0x0000FF;

    f = (double) amp * 0.01;
    for (i = 0; i < 256; i++) {
        c = round(f * i);
        lut[i] = (c >= 255.0) ? 255 : (ubyte) c;
    }

    for (i = 0; i < len; i++) {
        offset = 3 * i;
        v = lut[im[offset + ch]];
        // use max value when composite channels, see imagej
        buf[offset + 0] = MAX(buf[offset + 0], (r == 0) ? 0 : v);
        buf[offset + 1] = MAX(buf[offset + 1], (g == 0) ? 0 : v);
        buf[offset + 2] = MAX(buf[offset + 2], (b == 0) ? 0 : v);
    }
}


/* amp_x is amplification in percent */
void fluor_filter(ubyte *im, int w, int h, int amp_b, int color_b, int amp_g,
                  int color_g, int amp_r, int color_r)
{
    int64_t i;
    int64_t len = w * h * 3;
    ubyte *buf;

    if (amp_b == 100 && amp_g == 100 && amp_r == 100 &&
        color_b == BLU && color_g == GRN && color_r == RED) {
        return;
    }

    buf = (ubyte *) calloc(len, sizeof(ubyte));
    if (!buf) {
        fprintf(stderr, "debug: calloc failed\n");
    }

    apply_filter_one_channel(im, w, h, buf, amp_b, color_b, 2);
    apply_filter_one_channel(im, w, h, buf, amp_g, color_g, 1);
    apply_filter_one_channel(im, w, h, buf, amp_r, color_r, 0);

    for (i = 0; i < len; i++)
        im[i] = buf[i];

    free(buf);
}
