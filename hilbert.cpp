// Based on Python code from https://github.com/jakubcerveny/gilbert/blob/master/gilbert2d.py

#include <vector>

#include <cstdlib>

using std::vector;
using std::pair;
using std::make_pair;

#include "hilbert.h"

template<class T>
T sgn(const T &x) { return (x > 0) - (x < 0); }

template<class T>
pair<T, T> sgn(const pair<T, T> &pt) { return make_pair(sgn(pt.first), sgn(pt.second)); }

template<class T>
pair<T, T> operator+(const pair<T, T> &a, const pair<T, T> &b) { return make_pair(a.first + b.first, a.second + b.second); }

template<class T>
pair<T, T> operator+=(pair<T, T> &a, const pair<T, T> &b) {
    a.first += b.first;
    a.second += b.second;
    return a;
}

template<class T>
pair<T, T> operator-(const pair<T, T> &a, const pair<T, T> &b) { return make_pair(a.first - b.first, a.second - b.second); }

template<class T>
pair<T, T> operator-(const pair<T, T> &a) { return make_pair(-a.first, -a.second); }

template<class T>
pair<T, T> operator/(const pair<T, T> &a, const T &b) { return make_pair(a.first / b, a.second / b); }

static void gilbert2d(pt_t pt, pt_t a, pt_t b, curve_t &curve) {
//    """
//    Generalized Hilbert ('gilbert') space-filling curve for arbitrary-sized
//    2D rectangular grids.
//    """

    int w = abs(a.first + a.second);
    int h = abs(b.first + b.second);

    pt_t da = sgn(a); // unit major direction
    pt_t db = sgn(b); // unit orthogonal direction

    if (h == 1) {
        // trivial row fill
        for (int i = 0; i < w; i++) {
            curve.emplace_back(pt);
            pt += da;
        }
        return;
    }

    if (w == 1) {
        // trivial column fill
        for (int i = 0; i < h; i++) {
            curve.emplace_back(pt);
            pt += db;
        }
        return;
    }

    pt_t a2 = a / 2;
    pt_t b2 = b / 2;

    int w2 = abs(a2.first + a2.second);
    int h2 = abs(b2.first + b2.second);

    if (2 * w > 3 * h) {
        if ((w2 % 2) && (w > 2)) {
            // prefer even steps
            a2 += da;
        }
        // long case: split in two parts only
        gilbert2d(pt, a2, b, curve);
        gilbert2d(pt + a2, a - a2, b, curve);
    } else {
        if ((h2 % 2) && (h > 2)) {
            // prefer even steps
            b2 += db;
        }
        // standard case: one step up, one long horizontal, one step down
        gilbert2d(pt, b2, a2, curve);
        gilbert2d(pt + b2, a, b - b2, curve);
        gilbert2d(pt + (a - da) + (b2 - db),
                  -b2, -(a - a2), curve);
    }
}


void gilbert2d(int width, int height, curve_t &curve) {
    curve.clear();

    if (width >= height) {
        gilbert2d(make_pair(0, 0), make_pair(width, 0), make_pair(0, height), curve);
    } else {
        gilbert2d(make_pair(0, 0), make_pair(0, height), make_pair(width, 0), curve);
    }
}
