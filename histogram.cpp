/*
 * Copyright (c) 2015, 2017, 2020 Kent A. Vander Velden, kent.vandervelden@gmail.com
 *
 * This file is part of BinVis.
 *
 *     BinVis is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     BinVis is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with BinVis.  If not, see <https://www.gnu.org/licenses/>.
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


/// string_to_histo_dtype returns a histo_dtype_t type corresponding to the type named type.
/// @param [in] s The name of the type
/// @return The associated histo_dtype_t type.
histo_dtype_t string_to_histo_dtype(const std::string &s) {
    histo_dtype_t t;

    if (s == "U8") t = u8;
    else if (s == "U12") t = u12;
    else if (s == "U16") t = u16;
    else if (s == "U32") t = u32;
    else if (s == "U64") t = u64;
    else if (s == "F32") t = f32;
    else if (s == "F64") t = f64;
    else t = none;

    return t;
}


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

/// generate_histo computes the histogram for each byte within dat_u8.
/// @param [in] dat_u8 Byte data to be analyzed.
/// @param [in] n Length of dat_u8 in bytes
/// @return The calculated histogram of each byte of dat_u8, as vector of length 256 scaled between [0., 1.]
float *generate_histo(const unsigned char *dat_u8, long n) { //, histo_dtype_t dtype) {
    auto hist = new float[256];
    memset(hist, 0, sizeof(hist[0]) * 256);

    //if(dtype != u8) {
    //  abort()
    //}

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

    return hist;
}

/// generate_histo_2d computes a 2d histogram of each overlapping digram within dat_u8.
/// @param [in] dat_u8 Byte data to be analyzed.
/// @param [in] n Length of dat_u8 in bytes.
/// @param [in] dtype The type of data to cast dat_u8 as.
/// @return The 2d histogram, as a linearized matrix of size 256 * 256, containing counts of each digram,
int *generate_histo_2d(const unsigned char *dat_u8, long n, histo_dtype_t dtype) {
    auto hist = new int[256 * 256];
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
            break;
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

template<class T>
void hist_float_helper_3d(int *hist, T *dat_f, long n, int st) {
    for (long i = 0; i < n / long(sizeof(T)) - 2; i += st) {
        int a1;
        int a2;
        int a3;
        if (sizeof(T) == 4) {
            a1 = ((dat_f[i + 0] / FLT_MAX) * 255. + 255.) / 2.;
            a2 = ((dat_f[i + 1] / FLT_MAX) * 255. + 255.) / 2.;
            a3 = ((dat_f[i + 2] / FLT_MAX) * 255. + 255.) / 2.;
        } else if (sizeof(T) == 8) {
            a1 = ((dat_f[i + 0] / DBL_MAX) * 255. + 255.) / 2.;
            a2 = ((dat_f[i + 1] / DBL_MAX) * 255. + 255.) / 2.;
            a3 = ((dat_f[i + 2] / DBL_MAX) * 255. + 255.) / 2.;
        } else {
            abort();
        }

        if (isnan(dat_f[i + 0])) a1 = 255;
        if (isnan(dat_f[i + 1])) a2 = 255;
        if (isnan(dat_f[i + 2])) a3 = 255;

        if (isnan(dat_f[i + 0])) { if (signbit(dat_f[i + 0])) { a1 = 0; } else { a1 = 255; }}
        if (isnan(dat_f[i + 1])) { if (signbit(dat_f[i + 1])) { a2 = 0; } else { a2 = 255; }}
        if (isnan(dat_f[i + 2])) { if (signbit(dat_f[i + 2])) { a3 = 0; } else { a3 = 255; }}

        if (isinf(dat_f[i + 0])) { if (signbit(dat_f[i + 0])) { a1 = 0; } else { a1 = 255; }}
        if (isinf(dat_f[i + 1])) { if (signbit(dat_f[i + 1])) { a2 = 0; } else { a2 = 255; }}
        if (isinf(dat_f[i + 2])) { if (signbit(dat_f[i + 2])) { a3 = 0; } else { a3 = 255; }}

/*
    if(a1 < 0 || a1 > 255) {
      printf("0 %d %f\n", a1, dat_f[i+0]);
    }
    if(a3 < 0 || a3 > 255) {
      printf("2 %d %f\n", a3, dat_f[i+2]);
    }
    if(a2 < 0 || a2 > 255) {
      printf("1 %d %f %d %d\n", a2, dat_f[i+1], isnan(dat_f[i+1]), dat_f[i+1] < 0);
    }
*/

        if (a1 < 0) a1 = 0;
        if (a2 < 0) a2 = 0;
        if (a3 < 0) a3 = 0;

        if (a1 > 255) a1 = 255;
        if (a2 > 255) a2 = 255;
        if (a3 > 255) a3 = 255;

        hist[a1 * 256 * 256 + a2 * 256 + a3]++;
    }
}

/// generate_histo_3d computes a 3d histogram of each overlapping digram within dat_u8.
/// @param [in] dat_u8 Byte data to be analyzed.
/// @param [in] n Length of dat_u8 in bytes.
/// @param [in] dtype The type of data to cast dat_u8 as.
/// @param [in] overlap Whether to move by a single byte (true) or length of dtype (false) (not implemented correctly.)
/// @return The 2d histogram, as a linearized matrix of size 256 * 256, containing counts of each digram,
int *generate_histo_3d(const unsigned char *dat_u8, long n, histo_dtype_t dtype, bool overlap) {
    auto hist = new int[256 * 256 * 256];
    memset(hist, 0, sizeof(hist[0]) * 256 * 256 * 256);

    int st = overlap ? 1 : 3;

    switch (dtype) {
        case none:
            break;
        case u8: {
            for (long i = 0; i < n - 2; i += st) {
                int a1 = dat_u8[i + 0];
                int a2 = dat_u8[i + 1];
                int a3 = dat_u8[i + 2];

                hist[a1 * 256 * 256 + a2 * 256 + a3]++;
            }
        }
            break;
        case u12: {
            auto dat_u16 = (const unsigned short *) dat_u8;
            for (long i = 0; i < n / 2 - 2; i += st) {
                int a1 = (dat_u16[i + 0] & 0x0fff) / float(0x0fff) * 255.;
                int a2 = (dat_u16[i + 1] & 0x0fff) / float(0x0fff) * 255.;
                int a3 = (dat_u16[i + 2] & 0x0fff) / float(0x0fff) * 255.;

                hist[a1 * 256 * 256 + a2 * 256 + a3]++;
            }
        }
            break;
        case u16: {
            auto dat_u16 = (const unsigned short *) dat_u8;
            for (long i = 0; i < n / 2 - 2; i += st) {
                int a1 = dat_u16[i + 0] / float(0xffff) * 255.;
                int a2 = dat_u16[i + 1] / float(0xffff) * 255.;
                int a3 = dat_u16[i + 2] / float(0xffff) * 255.;

                hist[a1 * 256 * 256 + a2 * 256 + a3]++;
            }
        }
            break;
        case u32: {
            const unsigned int *dat_u32 = (const unsigned int *) dat_u8;
            for (long i = 0; i < n / 4 - 2; i += st) {
                int a1 = dat_u32[i + 0] / float(0xffffffff) * 255.;
                int a2 = dat_u32[i + 1] / float(0xffffffff) * 255.;
                int a3 = dat_u32[i + 2] / float(0xffffffff) * 255.;

                hist[a1 * 256 * 256 + a2 * 256 + a3]++;
            }
        }
            break;
        case u64: {
            auto dat_u64 = (const unsigned long *) dat_u8;
            for (long i = 0; i < n / 8 - 2; i += st) {
                int a1 = dat_u64[i + 0] / float(0xffffffffffffffff) * 255.;
                int a2 = dat_u64[i + 1] / float(0xffffffffffffffff) * 255.;
                int a3 = dat_u64[i + 2] / float(0xffffffffffffffff) * 255.;

                hist[a1 * 256 * 256 + a2 * 256 + a3]++;
            }
        }
            break;
        case f32: {
            auto dat_f32 = (const float *) dat_u8;
            hist_float_helper_3d(hist, dat_f32, n, st);
        }
            break;
        case f64: {
            auto dat_f64 = (const double *) dat_u8;
            hist_float_helper_3d(hist, dat_f64, n, st);
        }
            break;
    }

#if 0
    n_vertices = 0;
    float m=10000000, M=-1, a=0.;
    for(int i=0; i<256*256*256; i++) {
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

/// generate_entropy computes the entropy within bs-sized blocks of dat_u8.
/// @param [in] dat_u8 Byte data to be analyzed.
/// @param [in] n Length of dat_u8 in bytes.
/// @param [out] rv_len The length of the return vector.
/// @param [in] bs The block sized used to analyze dat_u8.
/// @return The calculated entropy for each block of dat_u8, as vector of length rv_len scaled between [0., 1.]
float *generate_entropy(const unsigned char *dat_u8, long n, long &rv_len, int bs) { //, histo_dtype_t dtype) {
    if (n <= 0) {
        rv_len = 0;
        return nullptr;
    }

    int inc = bs; // set to a value less than bs to create overlapping

    long ddn = n / inc + (n % inc ? 1 : 0);
    auto dd = new float[ddn];
    memset(dd, 0, sizeof(dd[0]) * ddn);

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

    rv_len = ddn;

    return dd;
}
