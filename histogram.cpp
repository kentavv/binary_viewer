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

#include <cfloat>
#include <cmath>
#include <algorithm>

#include <cstring>
#include <cstdlib>

#include "histogram.h"

using std::min;
using std::max;
using std::isinf;
using std::isnan;
using std::signbit;


template<class T>
void hist_float_helper_2d(int *hist, T *dat_f, long n) {
    for (long i = 0; i < n / long(sizeof(T)) - 1; i++) {
        int a1;
        int a2;
        if (sizeof(T) == 4) {
            a1 = ((dat_f[i + 0] / FLT_MAX) * 255. + 255.) / 2.;
            a2 = ((dat_f[i + 1] / FLT_MAX) * 255. + 255.) / 2.;
        } else if (sizeof(T) == 8) {
            a1 = ((dat_f[i + 0] / DBL_MAX) * 255. + 255.) / 2.;
            a2 = ((dat_f[i + 1] / DBL_MAX) * 255. + 255.) / 2.;
        } else {
            abort();
        }

        if (isnan(dat_f[i + 0])) a1 = 255;
        if (isnan(dat_f[i + 1])) a2 = 255;

        if (isnan(dat_f[i + 0])) { if (signbit(dat_f[i + 0])) { a1 = 0; } else { a1 = 255; }}
        if (isnan(dat_f[i + 1])) { if (signbit(dat_f[i + 1])) { a2 = 0; } else { a2 = 255; }}

        if (isinf(dat_f[i + 0])) { if (signbit(dat_f[i + 0])) { a1 = 0; } else { a1 = 255; }}
        if (isinf(dat_f[i + 1])) { if (signbit(dat_f[i + 1])) { a2 = 0; } else { a2 = 255; }}

#if 0
        if(a1 < 0 || a1 > 255) {
          printf("0 %d %f\n", a1, dat_f[i+0]);
        }
        if(a2 < 0 || a2 > 255) {
          printf("1 %d %f %d %d\n", a2, dat_f[i+1], isnan(dat_f[i+1]), dat_f[i+1] < 0);
        }
#endif

        if (a1 < 0) a1 = 0;
        if (a2 < 0) a2 = 0;

        if (a1 > 255) a1 = 255;
        if (a2 > 255) a2 = 255;

        hist[a1 * 256 + a2]++;
    }
}

int *generate_histo_2d(const unsigned char *dat_u8, long n, histo_dtype_t dtype) {
    int *hist = new int[256 * 256];
    memset(hist, 0, sizeof(hist[0]) * 256 * 256);

    switch (dtype) {
        case none:
            break;
        case u8: {
            for (long i = 0; i < n - 1; i++) {
                int a1 = dat_u8[i + 0];
                int a2 = dat_u8[i + 1];

                hist[a1 * 256 + a2]++;
            }
        }
            break;
        case u16: {
            auto dat_u16 = (const unsigned short *) dat_u8;
            for (long i = 0; i < n / 2 - 1; i++) {
                int a1 = dat_u16[i + 0] / float(0xffff) * 255.;
                int a2 = dat_u16[i + 1] / float(0xffff) * 255.;

                hist[a1 * 256 + a2]++;
            }
        }
            break;
        case u32: {
            auto dat_u32 = (const unsigned int *) dat_u8;
            for (long i = 0; i < n / 4 - 1; i++) {
                int a1 = dat_u32[i + 0] / float(0xffffffff) * 255.;
                int a2 = dat_u32[i + 1] / float(0xffffffff) * 255.;

                hist[a1 * 256 + a2]++;
            }
        }
            break;
        case u64: {
            auto dat_u64 = (const unsigned long *) dat_u8;
            for (long i = 0; i < n / 8 - 1; i++) {
                int a1 = dat_u64[i + 0] / float(0xffffffffffffffff) * 255.;
                int a2 = dat_u64[i + 1] / float(0xffffffffffffffff) * 255.;

                hist[a1 * 256 + a2]++;
            }
        }
            break;
        case f32: {
            auto dat_f32 = (const float *) dat_u8;
            hist_float_helper_2d(hist, dat_f32, n);
        }
        case f64: {
            auto dat_f64 = (const double *) dat_u8;
            hist_float_helper_2d(hist, dat_f64, n);
        }
            break;
    }

#if 0
    int n_vertices = 0;
    float m=10000000, M=-1, a=0.;
    for(int i=0; i<256*256; i++) {
      if(hist[i] > 0) {
        n_vertices++;

        if(m > hist[i]) m = hist[i];
        if(M < hist[i]) M = hist[i];
        a += hist[i];
      }
    }
    a /= n_vertices;
    printf("%d %f %f %f\n", n_vertices, m, M, a);
#endif

    return hist;
}

float *generate_histo(const unsigned char *dat_u8, long n) { //, histo_dtype_t dtype) {
    auto hist = new float[256];
    memset(hist, 0, sizeof(hist[0]) * 256);

    //if(dtype != u8) {
    //  abort()
    //}

    {
        for (long i = 0; i < n; i++) {
            hist[dat_u8[i]]++;
        }

        float mx = 0.;
        for (int i = 0; i < 256; i++) {
            mx = max(mx, hist[i]);
        }
        for (int i = 0; i < 256; i++) {
            hist[i] /= mx;
        }
    }

    return hist;
}

float *generate_entropy(const unsigned char *dat_u8, long n, long &rv_len, int bs) { //, histo_dtype_t dtype) {
    if (n <= 0) {
        rv_len = 0;
        return nullptr;
    }

    int inc = bs; // set to a value less than bs to create overlapping

    long ddn = n / inc + (n % inc ? 1 : 0);
    auto dd = new float[ddn];
    memset(dd, 0, sizeof(dd[0]) * ddn);

    {
        for (long is = 0; is < n; is += inc) {
            long ie = min(n, is + bs);

            int dict[256] = {0};
            for (long i = is; i < ie; i++) {
                dict[dat_u8[i]]++;
            }

            float entropy = 0.;
            for (int i = 0; i < 256; i++) {
                float p = dict[i] / float(ie - is);
                if (p > 0.) {
                    entropy += -p * logf(p);
                }
            }
            entropy /= logf(2.0);
            entropy /= 8.0;

            int di = is / bs;
            if (di >= ddn) {
                //printf("%d %d %d %d\n", is, bs, is/bs, n);
                continue;
            }

            dd[di] = entropy;
        }
    }

    rv_len = ddn;

    return dd;
}
