/*
 * MIT License
 * 
 * Copyright (c) 2015, 2017 Kent A. Vander Velden
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bayer.h"

// A nice description of Bayer demosaicing is at http://www.cambridgeincolour.com/tutorials/camera-sensors.htm
// Method 2 is based on this description

//static bool debug = false;

static unsigned int bayerEdgeHelp(const unsigned char *in, const int h, const int w, const int in_row_w, const int y, const int x, int &n) {
    if (0 <= y && y < h &&
        0 <= x && x < w) {
        n++;
        //if(debug) printf("p,n: %d %d\n", in[y * in_row_w + x], n);
        return in[y * in_row_w + x];
    }
    return 0;
}

static void
bayerBGPixel2(const unsigned char *in, const int h, const int w, const int in_row_w, const int out_row_w, int perm, const int y, const int x,
              unsigned char *out) {
    unsigned int r, g, b;
    int n;

    int all_perm[24][4] = {
            {0, 1, 2, 3},
            {0, 1, 3, 2},
            {0, 2, 1, 3},
            {0, 2, 3, 1},
            {0, 3, 1, 2},
            {0, 3, 2, 1},
            {1, 0, 2, 3},
            {1, 0, 3, 2},
            {1, 2, 0, 3},
            {1, 2, 3, 0},
            {1, 3, 0, 2},
            {1, 3, 2, 0},
            {2, 0, 1, 3},
            {2, 0, 3, 1},
            {2, 1, 0, 3},
            {2, 1, 3, 0},
            {2, 3, 0, 1},
            {2, 3, 1, 0},
            {3, 0, 1, 2},
            {3, 0, 2, 1},
            {3, 1, 0, 2},
            {3, 1, 2, 0},
            {3, 2, 0, 1},
            {3, 2, 1, 0}
    };

    if (perm < 0 || 24 <= perm) abort();

    int type = 0; // current pixel is R if 0, G0 if 1, G1 if 2, B if 3
    if ((y % 2) == 0 && (x % 2) == 0) {
        type = all_perm[perm][0];
    } else if ((y % 2) == 0 && (x % 2) == 1) {
        type = all_perm[perm][1];
    } else if ((y % 2) == 1 && (x % 2) == 0) {
        type = all_perm[perm][2];
    } else {
        type = all_perm[perm][3];
    }

    bool debug = false;

    if (debug) printf("x,y,v: %d %d %d\n", x, y, in[y * in_row_w + x]);
    if (debug) printf("type: %d\n", type);

    if (type == 0) {
        n = 0;
        r = bayerEdgeHelp(in, h, w, in_row_w, y, x, n);
        if (debug) printf("r,n=: %d %d\n", r, n);

        n = 0;
        g = (bayerEdgeHelp(in, h, w, in_row_w, y, x + 1, n) +
             bayerEdgeHelp(in, h, w, in_row_w, y + 1, x, n));
        if (n > 1) g /= n;
        if (debug) printf("g,n=: %d %d\n", g, n);

        n = 0;
        b = bayerEdgeHelp(in, h, w, in_row_w, y + 1, x + 1, n);
        if (debug) printf("b,n=: %d %d\n", b, n);
    } else if (type == 1) {
        n = 0;
        r = bayerEdgeHelp(in, h, w, in_row_w, y, x + 1, n);
        if (debug) printf("r,n=: %d %d\n", r, n);

        n = 0;
        g = (bayerEdgeHelp(in, h, w, in_row_w, y, x, n) +
             bayerEdgeHelp(in, h, w, in_row_w, y + 1, x + 1, n));
        if (n > 1) g /= n;
        if (debug) printf("g,n=: %d %d\n", g, n);

        n = 0;
        b = bayerEdgeHelp(in, h, w, in_row_w, y + 1, x, n);
        if (debug) printf("b,n=: %d %d\n", b, n);
    } else if (type == 2) {
        n = 0;
        r = bayerEdgeHelp(in, h, w, in_row_w, y + 1, x, n);
        if (debug) printf("r,n=: %d %d\n", r, n);

        n = 0;
        g = (bayerEdgeHelp(in, h, w, in_row_w, y, x, n) +
             bayerEdgeHelp(in, h, w, in_row_w, y + 1, x + 1, n));
        if (n > 1) g /= n;
        if (debug) printf("g,n=: %d %d\n", g, n);

        n = 0;
        b = bayerEdgeHelp(in, h, w, in_row_w, y, x + 1, n);
        if (debug) printf("b,n=: %d %d\n", b, n);
    } else {
        n = 0;
        r = bayerEdgeHelp(in, h, w, in_row_w, y + 1, x + 1, n);
        if (debug) printf("r,n=: %d %d\n", r, n);

        n = 0;
        g = (bayerEdgeHelp(in, h, w, in_row_w, y, x + 1, n) +
             bayerEdgeHelp(in, h, w, in_row_w, y + 1, x, n));
        if (n > 0) g /= n;
        if (debug) printf("g,n=: %d %d\n", g, n);

        n = 0;
        b = bayerEdgeHelp(in, h, w, in_row_w, y, x, n);
        if (debug) printf("b,n=: %d %d\n", b, n);
    }

    // The R and B values are exchanged, and does not makes sense
    out[y * out_row_w + x * 3 + 0] = r;
    out[y * out_row_w + x * 3 + 1] = g;
    out[y * out_row_w + x * 3 + 2] = b;
}

void bayerBG(const unsigned char *bayer, const int h, const int w, const int bayer_row_w, int perm, unsigned char *rgb, const int rgb_row_w) {
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            bayerBGPixel2(bayer, h, w, bayer_row_w, rgb_row_w, perm, y, x, rgb);
        }
    }
}

void bayerBG(const unsigned char *bayer, const int h, const int w, int perm, unsigned char *rgb) {
    bayerBG(bayer, h, w, w, perm, rgb, w * 3);
}

