#include "extern.h"
#include <math.h>


void resize_mask(bool *mask, int mw, int mh, bool *outim, int w, int h)
{
    int mlen = mw * mh;
    int i, j;
    int x, y, mx;
    double f = (double) mw / w;
    int len = w * h;

    for (i = 0; i < len - 1; i++) {
        /* map (x, y) on output canvas to (x, y) on input mask */
        y = i / w;
        x = i - y * w;
        mx = round(x * f);
        if (mx >= mw)
            mx = mw - 1;

        j = round(y * f) * mw + mx;
        if (j >= mlen)
            continue;

        outim[i] = mask[j] == 1 ? 1 : 0;
    }
}


/* mark the border of mask as white */
void highlight_mask_border(ubyte *mask, int w, int h, uint32_t *out_area,
                           uint32_t *out_border_len)
{
    int len = w * h;
    int up, down, left, right;
    int ul, ur, dl, dr;
    int idx_r0, idx_ru, idx_rd;

    int i = -1;
    idx_r0 = 0;

    // not sure whether it is faster to increment passed in *area directly
    uint32_t area = 0;
    uint32_t border_len = 0;
    for (int r = 0; r < h; r++) {
        idx_ru = idx_r0 - w;
        idx_rd = idx_r0 + w;

        for (int c = 0; c < w; c++) {
            i++;
            if (mask[i] == 0)
                continue;

            area++;
            // check 8-ways to find a background pixel
            up = i - w;
            if (up >= 0 && mask[up] == 0) {
                mask[i] = 255;
                border_len++;
                continue;
            }

            down = i + w;
            if (down < len && mask[down] == 0) {
                mask[i] = 255;
                border_len++;
                continue;
            }

            left = i - 1;
            if (left >= idx_r0 && mask[left] == 0) {
                mask[i] = 255;
                border_len++;
                continue;
            }

            right = i + 1;
            if (right < idx_rd && mask[right] == 0) {
                mask[i] = 255;
                border_len++;
                continue;
            }

            ul = up - 1;
            if (ul >= 0 && ul >= idx_ru && mask[ul] == 0) {
                mask[i] = 255;
                border_len++;
                continue;
            }

            ur = up + 1;
            if (ur >= 0 && ur < idx_r0 && mask[ur] == 0) {
                mask[i] = 255;
                border_len++;
                continue;
            }

            dl = down - 1;
            if (dl < len && dl >= idx_rd && mask[dl] == 0) {
                mask[i] = 255;
                border_len++;
                continue;
            }

            dr = down - 1;
            if (dr < len && mask[dl] == 0) {
                mask[i] = 255;
                border_len++;
                continue;
            }
        }

        idx_r0 += w;
    }

    *out_area = area;
    *out_border_len = border_len;
}
