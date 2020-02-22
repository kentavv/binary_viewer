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
pair<T, T> operator+=(pair<T, T> &a, const pair<T, T> &b) { a.first += b.first; a.second += b.second; return a; }

template<class T>
pair<T, T> operator-(const pair<T, T> &a, const pair<T, T> &b) { return make_pair(a.first - b.first, a.second - b.second); }

template<class T>
pair<T, T> operator-(const pair<T, T> &a) { return make_pair(-a.first, -a.second); }

template<class T>
pair<T, T> operator/(const pair<T, T> &a, const T &b) { return make_pair(a.first / b, a.second / b); }

static void gilbert2d(pt_t pt, pt_t a, pt_t b, curve_t &curve) {

//def gilbert2d(x, y, ax, ay, bx, by):
//
//    """
//    Generalized Hilbert ('gilbert') space-filling curve for arbitrary-sized
//    2D rectangular grids.
//    """
//
//    w = abs(ax + ay)
//    h = abs(bx + by)
    int w = abs(a.first + a.second);
    int h = abs(b.first + b.second);

//
//    (dax, day) = (sgn(ax), sgn(ay)) # unit major direction
//    (dbx, dby) = (sgn(bx), sgn(by)) # unit orthogonal direction
//    pair<int, int> da = make_pair(sgn(a.first), sgn(a.second)); // unit major direction
//    pair<int, int> db = make_pair(sgn(b.first), sgn(b.second)); // unit orthogonal direction
    pt_t da = sgn(a); // unit major direction
    pt_t db = sgn(b); // unit orthogonal direction

//
//    if h == 1:
//        # trivial row fill
//        for i in range(0, w):
//            print x, y
//            (x, y) = (x + dax, y + day)
//        return
    if (h == 1) {
        // trivial row fill
        for (int i = 0; i < w; i++) {
            curve.emplace_back(pt);
//            pt = make_pair(pt.first + da.first, pt.second + da.second);
            pt += da;
        }
        return;
    }

//
//    if w == 1:
//        # trivial column fill
//        for i in range(0, h):
//            print x, y
//            (x, y) = (x + dbx, y + dby)
//        return
    if (w == 1) {
        // trivial column fill
        for (int i = 0; i < h; i++) {
            curve.emplace_back(pt);
//            pt = make_pair(pt.first + db.first, pt.second + db.second);
            pt += db;
        }
        return;
    }
//
//    (ax2, ay2) = (ax/2, ay/2)
//    (bx2, by2) = (bx/2, by/2)
    pt_t a2 = a / 2;
    pt_t b2 = b / 2;
//
//    w2 = abs(ax2 + ay2)
//    h2 = abs(bx2 + by2)
    int w2 = abs(a2.first + a2.second);
    int h2 = abs(b2.first + b2.second);
//
//    if 2*w > 3*h:
//        if (w2 % 2) and (w > 2):
//            # prefer even steps
//            (ax2, ay2) = (ax2 + dax, ay2 + day)
//
//        # long case: split in two parts only
//        gilbert2d(x, y, ax2, ay2, bx, by)
//        gilbert2d(x+ax2, y+ay2, ax-ax2, ay-ay2, bx, by)
    if (2 * w > 3 * h) {
        if ((w2 % 2) && (w > 2)) {
            // prefer even steps
            a2 += da;
        }
        // long case: split in two parts only
        gilbert2d(pt, a2, b, curve);
        gilbert2d(pt + a2,
                  a - a2,
                  b,
                  curve);
    }
//
//    else:
//        if (h2 % 2) and (h > 2):
//            # prefer even steps
//            (bx2, by2) = (bx2 + dbx, by2 + dby)
//
//        # standard case: one step up, one long horizontal, one step down
//        gilbert2d(x, y, bx2, by2, ax2, ay2)
//        gilbert2d(x+bx2, y+by2, ax, ay, bx-bx2, by-by2)
//        gilbert2d(x+(ax-dax)+(bx2-dbx), y+(ay-day)+(by2-dby),
//                 -bx2, -by2, -(ax-ax2), -(ay-ay2))
    else {
        if ((h2 % 2) && (h > 2)) {
            // prefer even steps
            b2 += db;
        }
        // standard case: one step up, one long horizontal, one step down
        gilbert2d(pt, b2, a2, curve);
        gilbert2d(pt + b2,
                  a,
                  b - b2,
                  curve);
        gilbert2d(pt + (a - da) + (b2 - db),
                  -b2,
                  -(a - a2),
                  curve);
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
