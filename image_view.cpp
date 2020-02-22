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

#include <unistd.h>

#include <QtGui>

#include "hilbert.h"
#include "image_view.h"

using std::max;
using std::min;
using std::vector;
using std::pair;
using std::make_pair;

ImageView::ImageView(QWidget *p)
        : QLabel(p),
          m1_(0.), m2_(1.), s_(none), allow_selection_(true) {
}

ImageView::~ImageView() {
}

void ImageView::enableSelection(bool v) {
    allow_selection_ = v;
    update();
}

void ImageView::setImage(QImage &img) {
    img_ = img;

    update_pix();

    update();
}

void ImageView::set_data(const unsigned char *dat, long len) {
    vector<pair<int, int> > hilbert;

    int w = width();
    int h = height();

    m1_ = 0.;
    m2_ = 1.;

    {
        int wh = w * h;

//        printf("wxh: %d %d\n", w, h);

        int h_ind = 0;

        if (len <= wh) {
            int img_w = w, img_h = len / w + 1;
            QImage img(img_w, img_h, QImage::Format_RGB32);
            img.fill(0);

            gilbert2d(img_w, img_h, hilbert);

            auto p = (unsigned int *) img.bits();

#if 0
            for (int i = 0; i < len; i++) {
                unsigned char c = dat[i];
                unsigned char r = 20;
                unsigned char g = c;
                unsigned char b = 20;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
#else
            for (int i = 0; i < len; i++) {
                unsigned char c = dat[i];
                unsigned char r, g, b;
                if (c == 0x00) {
                    r = 0x00;
                    g = 0x00;
                    b = 0x00;
                } else if (0x00 < c && c <= 0x1f) {
                    r = 0x00;
                    g = 0x00;
                    b = 0xf0;
                } else if (0x1f < c && c <= 0x7f) {
                    r = 0x00;
                    g = 0xf0;
                    b = 0x00;
                } else if (0x7f < c && c < 0xff) {
                    r = 0xf0;
                    g = 0x00;
                    b = 0x00;
                } else if (c == 0xff) {
                    r = 0xff;
                    g = 0xff;
                    b = 0xff;
                }
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
#if 0
                *p++ = v;
#else
                {
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
#endif
            }
#endif
//            printf("A %d %d %d %d\n", width(), height(), img.width(), img.height());
            img = img.scaled(size());
            setImage(img);
        } else {
            int sf = len / wh + 1;
            int img_w = w, img_h = len / sf / w + 1;
            QImage img(img_w, img_h, QImage::Format_RGB32);
            img.fill(0);

            gilbert2d(img_w, img_h, hilbert);

            auto p = (unsigned int *) img.bits();

#if 0
            for (int i = 0; i < len;) {
                unsigned char c;
                int cn = 0;
                int j;
                for (j = 0; i < len && j < sf; i++, j++) {
                    cn += dat[i];
                }
                c = cn / j;
                unsigned char r = 20;
                unsigned char g = c;
                unsigned char b = 20;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
#else
            for (int i = 0; i < len;) {
                int r = 0, g = 0, b = 0;
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

                r = min(255, r / j) & 0xff;
                g = min(255, g / j) & 0xff;
                b = min(255, b / j) & 0xff;

                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
#if 0
                *p++ = v;
#else
                {
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
#endif
            }
#endif
//            printf("B %d %d %d %d\n", width(), height(), img.width(), img.height());
            img = img.scaled(size());
            setImage(img);
        }
    }
}

void ImageView::paintEvent(QPaintEvent *e) {
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

    {
        // a border around the image helps to see the border of a dark image
        p.setPen(Qt::darkGray);
        p.drawRect(0, 0, width() - 1, height() - 1);
    }
}

void ImageView::resizeEvent(QResizeEvent *e) {
    QLabel::resizeEvent(e);

    update_pix();
}

void ImageView::update_pix() {
    if (img_.isNull()) return;

    int vw = width() - 4;
    int vh = height() - 4; // TODO BUG: With QDarkStyle, without the subtraction, the height or width of the application grows without bounds.
    pix_ = QPixmap::fromImage(img_).scaled(vw, vh); //, Qt::KeepAspectRatio);
    setPixmap(pix_);
    printf("%d %d   %d %d   %d %d\n", vw, vh, img_.width(), img_.height(), width(), height());
}

void ImageView::mousePressEvent(QMouseEvent *e) {
    e->accept();

    if (!allow_selection_) return;
    if (e->button() != Qt::LeftButton) return;

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

void ImageView::mouseMoveEvent(QMouseEvent *e) {
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

void ImageView::mouseReleaseEvent(QMouseEvent *e) {
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
