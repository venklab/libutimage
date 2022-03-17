#include "extern.h"

#define MAXPOINTS 8192
#define MIN(a,b) (((a)<(b))?(a):(b))


struct mm_global {
    int len;
    int v;
    uint64_t count;   // grow pixels count
    uint64_t *vcounts;  // pixel count for each island before grow
    ubyte *imbuf;
};

static struct mm_global gROOT;


static void merge_imbuf(ubyte *imdata, ubyte *imbuf, int len)
{
    for (int i = 0; i < len; i++) {
        if (imbuf[i] != 0) {
            imdata[i] = imbuf[i];
            imbuf[i] = 0;
        }
    }
}

/* dilate imdata for one pixel, check overlapping with mask
 * Return: 0 if not overlap with mask after dilate
 *         1 if overlap with mask after dilate
 *        -1 on error
 */
static int grow_one_pixel_until_mask(ubyte *imdata, ubyte *mask,
                                     int w, int idx)
{
    int up, down, right, left;
    int upleft, dwleft, upright, dwright;
    int r1, r2;  // row begin and row end
    int v;

    if (idx >= gROOT.len || idx < 0)
        return -1;

    v = imdata[idx];

    r1 = (idx / w) * w;
    r2 = r1 + w;
    left = idx - 1;
    right = idx + 1;

    // grow by C8 connection
    if (left >= r1) {
        if (mask[left] == 0 && imdata[left] == 0) {
            imdata[left] = v;
            gROOT.count++;
        } else if (mask[left] != 0) {
            return 1;
        }

        upleft = left - w;
        if (upleft >= 0 && mask[upleft] == 0 && imdata[upleft] == 0) {
            imdata[upleft] = v;
            gROOT.count++;
        } else if (mask[upleft] != 0) {
            return 1;
        }

        dwleft = left + w;
        if (dwleft < gROOT.len && mask[dwleft] == 0 && imdata[dwleft] == 0) {
            gROOT.imbuf[dwleft] = v;
            gROOT.count++;
        } else if (mask[dwleft] != 0) {
            return 1;
        }
    }

    if (right < r2) {
        if (mask[right] == 0 && imdata[right] == 0) {
            gROOT.imbuf[right] = v;
            gROOT.count++;
        } else if (mask[right] != 0) {
            return 1;
        }

        upright = right - w;
        if (upright > 0 && mask[upright] == 0 && imdata[upright] == 0) {
            imdata[upright] = v;
            gROOT.count++;
        } else if (mask[upright] != 0) {
            return 1;
        }

        dwright = right + w;
        if (dwright < gROOT.len && mask[dwright] == 0 && imdata[dwright] == 0) {
            gROOT.imbuf[dwright] = v;
            gROOT.count++;
        } else if (mask[dwright] != 0) {
            return 1;
        }
    }

    up = idx - w;
    if (up >=0 && mask[up] == 0 && imdata[up] == 0) {
        imdata[up] = v;
        gROOT.count++;
    } else if (mask[up] != 0) {
        return 1;
    }

    down = idx + w;
    if (down < gROOT.len && mask[down] == 0 && imdata[down] == 0) {
        gROOT.imbuf[down] = v;
        gROOT.count++;
    } else if (mask[down] != 0) {
        return 1;
    }

    return 0;
}


/* mask 0 value is background
 * d is the diameter of narrowest location */
int distance_by_dilate(ubyte *mask1, ubyte *mask2, int w, int h)
{
    int len = w * h;
    int i;

    gROOT.len = len;
    gROOT.imbuf = calloc(len, sizeof(ubyte));

   // grow by C8 connection, one pixel on each direction a time. Use the number
   // of iterations as distance between two masks
   int dist = 0;
   while (1) {
        gROOT.count = 0;
        for (i = 0; i < len; i++) {
            if (mask1[i] != 0 &&
                1 == grow_one_pixel_until_mask(mask1, mask2, w, i))
                gROOT.count = 0;  // signal to break outer loop
        }

        if (gROOT.count == 0)
            break;

        merge_imbuf(mask1, gROOT.imbuf, len);
        dist++;
   }

   free(gROOT.imbuf);

   return dist;
}

