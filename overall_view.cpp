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

#include <QtGui>

#include "hilbert.h"
#include "overall_view.h"

using std::max;
using std::min;
using std::vector;
using std::pair;
using std::make_pair;

OverallView::OverallView(QWidget *p)
        : QLabel(p),
          m1_(0.), m2_(1.), px_(-1), py_(-1), s_(none), allow_selection_(true),
          use_byte_classes_(true),
          use_hilbert_curve_(true),
          dat_(nullptr), len_(0) {
}

void OverallView::enableSelection(bool v) {
    allow_selection_ = v;
    update();
}

void OverallView::setImage(QImage &img) {
    img_ = img;

    update_pix();

    update();
}

void OverallView::set_data(const unsigned char *dat, long len, bool reset_selection) {
    dat_ = dat;
    len_ = len;

    if (reset_selection) {
        m1_ = 0.;
        m2_ = 1.;
    }

    int w = width();
    int h = height();
    int wh = w * h;
    int sf = len / wh + 1;

    int img_w = w, img_h = len / sf / w + 1;
    QImage img(img_w, img_h, QImage::Format_RGB32);
    printf("%d %d   %d %d\n", w, h, img_w, img_h);
    img.fill(0);

    curve_t hilbert;
    int h_ind = 0;
    if (use_hilbert_curve_) gilbert2d(img_w, img_h, hilbert);

    auto p = (unsigned int *) img.bits();

    for (int i = 0; i < len;) {
        int r = 0, g = 0, b = 0;

        if (!use_byte_classes_) {
            int cn = 0;
            int j;
            for (j = 0; i < len && j < sf; i++, j++) {
                cn += dat[i];
            }
            r = 20;
            g = cn / j;
            b = 20;
        } else {
            int j;
            for (j = 0; i < len && j < sf; i++, j++) {
                unsigned char c = dat[i];
                if (c == 0x00) {
                    r += 0x00;
                    g += 0x00;
                    b += 0x00;
                } else if (0x00 < c && c <= 0x1f) {
                    r += 0x00;
                    g += 0x00;
                    b += 0xf0;
                } else if (0x1f < c && c <= 0x7f) {
                    r += 0x00;
                    g += 0xf0;
                    b += 0x00;
                } else if (0x7f < c && c < 0xff) {
                    r += 0xf0;
                    g += 0x00;
                    b += 0x00;
                } else if (c == 0xff) {
                    r += 0xff;
                    g += 0xff;
                    b += 0xff;
                }
            }

            r /= j;
            g /= j;
            b /= j;
        }

        r = min(255, r) & 0xff;
        g = min(255, g) & 0xff;
        b = min(255, b) & 0xff;

        unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);

        if (!use_hilbert_curve_) {
            *p++ = v;
        } else {
            if (h_ind >= hilbert.size()) abort();

            int x = hilbert[h_ind].first;
            int y = hilbert[h_ind++].second;
            int ind = y * w + x;
            if (ind < wh) {
                p[ind] = v;
            } else {
                abort();
            }
        }
    }

    img = img.scaled(size());
    setImage(img);
}

void OverallView::paintEvent(QPaintEvent *e) {
    QLabel::paintEvent(e);

    QPainter p(this);
    if (allow_selection_) {
        int ry1 = m1_ * height();
        int ry2 = m2_ * height();

        QBrush brush(QColor(128, 64, 64, 128 + 32));
        QPen pen(brush, 5, Qt::SolidLine, Qt::RoundCap);
        p.setPen(pen);
        p.drawLine(0 + 3, ry1, width() - 1 - 3, ry1);
        p.drawLine(0 + 3, ry2, width() - 1 - 3, ry2);
    }

    // a border around the image helps to see the border of a dark image
    p.setPen(Qt::darkGray);
    p.drawRect(0, 0, width() - 1, height() - 1);
}

void OverallView::resizeEvent(QResizeEvent *e) {
    QLabel::resizeEvent(e);

    update_pix();
}

void OverallView::update_pix() {
    if (img_.isNull()) return;

    int vw = width() - 4;
    int vh = height() - 4; // TODO BUG: With QDarkStyle, without the subtraction, the height or width of the application grows without bounds.
    pix_ = QPixmap::fromImage(img_).scaled(vw, vh); //, Qt::KeepAspectRatio);
    setPixmap(pix_);
    printf("%d %d   %d %d   %d %d\n", vw, vh, img_.width(), img_.height(), width(), height());
}

// Gray code related functions are from https://en.wikipedia.org/wiki/Gray_code
static unsigned int BinaryToGray(unsigned int num) {
    return num ^ (num >> 1);
}

static unsigned int GrayToBinary(unsigned int num) {
    unsigned int mask = num >> 1;
    while (mask != 0) {
        num = num ^ mask;
        mask = mask >> 1;
    }
    return num;
}

void OverallView::mousePressEvent(QMouseEvent *e) {
    e->accept();

    if (e->button() == Qt::RightButton) {
        unsigned char v = (use_byte_classes_ ? 0x02 : 0x00) | (use_hilbert_curve_ ? 0x01 : 0x00);
        v = BinaryToGray((GrayToBinary(v) + 1) & 0x03);
        use_byte_classes_ = v & 0x02;
        use_hilbert_curve_ = v & 0x01;
        set_data(dat_, len_, false);
        return;
    }

    if (e->button() == Qt::LeftButton) {
        if (!allow_selection_) return;

        int x = e->pos().x();
        int y = e->pos().y();

        if (x < 0) x = 0;
        if (x > width() - 1) x = width() - 1;
        if (y < 0) y = 0;
        if (y > height() - 1) y = height() - 1;

        float yp = y / float(height());

        if (yp > m1_ && (yp - m1_) < .01) {
            s_ = m1_moving;
        } else if (yp < m2_ && (m2_ - yp) < .01) {
            s_ = m2_moving;
        } else if (m1_ < yp && yp < m2_) {
            s_ = m12_moving;
        } else {
            s_ = none;
        }

        px_ = x;
        py_ = y;
    }
}

void OverallView::mouseMoveEvent(QMouseEvent *e) {
    e->accept();

    if (s_ == none) return;

    int x = e->pos().x();
    int y = e->pos().y();

    if (x < 0) x = 0;
    if (x > width() - 1) x = width() - 1;
    if (y < 0) y = 0;
    if (y > height() - 1) y = height() - 1;

    if (y == py_) return;

    int h = height();

    float m1 = m1_;
    float m2 = m2_;
    if (s_ == m1_moving) {
        m1 = y / float(h);
    } else if (s_ == m2_moving) {
        m2 = y / float(h);
    } else if (s_ == m12_moving) {
        float dy = (y - py_) / float(h);
        m1 += dy;
        m2 += dy;
    }
    if (m1 >= m2_ - .01) m1 = m2_ - .01;
    if (m1 < 0.) m1 = 0.;
    if (m2 <= m1_ + .01) m2 = m1_ + .01;
    if (m2 > 1.) m2 = 1.;

    m1_ = m1;
    m2_ = m2;

    px_ = x;
    py_ = y;

    update();

    emit(rangeSelected(m1_, m2_));
}

void OverallView::mouseReleaseEvent(QMouseEvent *e) {
    e->accept();

    if (e->button() != Qt::LeftButton) return;

//    int x = e->pos().x();
//    int y = e->pos().y();
//
//    if (x < 0) x = 0;
//    if (x > width() - 1) x = width() - 1;
//    if (y < 0) y = 0;
//    if (y > height() - 1) y = height() - 1;

    px_ = -1;
    py_ = -1;
    s_ = none;
}
