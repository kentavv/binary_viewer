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
#include <QGridLayout>
#include <QComboBox>
#include <QScrollBar>

#include "binary_viewer.h"


BinaryView::BinaryView(QWidget *p)
        : QWidget(p),
          dat_(nullptr), dat_n_(0), off_(0) {
}

int BinaryView::rowHeight() const {
    QFont font("Courier New", 24);
    QFontMetrics fm(font);

    int fh = fm.height();
    int fw = fm.maxWidth();

    return fh;
}

void BinaryView::paintEvent(QPaintEvent *e) {
    QWidget::paintEvent(e);

    QPainter p(this);

    int w = width();
    int h = height();

    // If the font is changed, update rowHeight()
    QFont font("Courier New", 24);
    p.setFont(font);
    QFontMetrics fm(font);

    int fh = fm.height();
    int fw = fm.maxWidth();

    int nvis_rows = h / fh;

    QPen default_pen = p.pen();

    for (int i = 0; i < nvis_rows; i++) {
        int x = 10;
        int y = (i + 1) * fh;

        long pos = (off_ + i) * 16;

        QString s1;
        s1 = QString("0x %1 %2").arg((pos >> 16) & 0xffff, 4, 16, QChar('0')).arg(pos & 0xffff, 4, 16, QChar('0'));

        p.setPen(default_pen);
        p.drawText(x, y, s1);

        x += (2 + 1 + 4 + 1 + 4) * fw;
        x += 4 * fw;
        int px = x;

        for (int j = 0; j < 16; j++) {
            if (pos + j >= dat_n_) break;

            if (j > 0) x += 1.2 * fw + 2 * fw;
            if (j == 16 / 2) x += 2 * fw;

            unsigned char c = dat_[pos + j];

            int r, g, b;
            if (c == 0x00) {
                r = 0x55;
                g = 0x55;
                b = 0x55;
            } else if (0x00 < c && c <= 0x1f) {
                r = 0x60;
                g = 0x60;
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
            p.setPen(QPen(QRgb(v)));

            QString s2;
            s2 = QString("%1").arg(c, 2, 16, QChar('0'));
            p.drawText(x, y, s2);
        }

        x = px + int(1.2 * fw + 2 * fw) * 16 + 2 * fw;
        x += 4 * fw;

        p.setPen(default_pen);
        for (int j = 0; j < 16; j++) {
            if (pos + j >= dat_n_) break;

            if (j > 0) x += 1.2 * fw + 1 * fw;
            if (j == 16 / 2) x += 2 * fw;

            unsigned char c = dat_[pos + j];

            if (!(0x20 <= c && c <= 0x7e)) {
                p.setPen(QPen(0xff606060));
                c = '.';
            } else {
                p.setPen(default_pen);
            }

            QString s2;
            s2 = QString("%1").arg(char(c));
            p.drawText(x, y, s2);
        }
    }

    // a border around the image helps to see the border of a dark image
    p.setPen(Qt::darkGray);
    p.drawRect(0, 0, width() - 1, height() - 1);
}

void BinaryView::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);

}

void BinaryView::setData(const unsigned char *dat, long n) {
    dat_ = dat;
    dat_n_ = n;
    off_ = 0;
    update();
}

void BinaryView::setStart(int off) {
    off_ = off;
    update();
}


BinaryViewer::BinaryViewer(QWidget *p)
        : QWidget(p)
//          hist_(nullptr), dat_(nullptr), dat_n_(0)
{
    auto layout = new QHBoxLayout(this);

    bv_ = new BinaryView();
    sb_ = new QScrollBar();

    layout->addWidget(bv_);
    layout->addWidget(sb_);

    sb_->setRange(0, 0);
    sb_->setFocus();
    sb_->setFocusPolicy(Qt::StrongFocus);

    connect(sb_, SIGNAL(valueChanged(int)), bv_, SLOT(setStart(int)));

    setLayout(layout);
}

BinaryViewer::~BinaryViewer() {
}

void BinaryViewer::paintEvent(QPaintEvent *e) {
    QWidget::paintEvent(e);
}

void BinaryViewer::resizeEvent(QResizeEvent *e) {
    QWidget::resizeEvent(e);

    int nvis_rows = height() / bv_->rowHeight();
    int page_step = std::max(16, nvis_rows - 2);
    sb_->setRange(0, int(ceil(dat_n_ / 16.)) - nvis_rows + 1);
    sb_->setPageStep(page_step);
}

void BinaryViewer::setData(const unsigned char *dat, long n) {
    dat_ = dat;
    dat_n_ = n;

    bv_->setData(dat, n);
    int nvis_rows = height() / bv_->rowHeight();
    sb_->setRange(0, int(ceil(dat_n_ / 16.)) - nvis_rows + 1);
}

void BinaryViewer::setStart(int s) {
    sb_->setValue(s);
}

void BinaryViewer::enterEvent(QEvent *e) {
    QWidget::enterEvent(e);
    sb_->setFocus();
}

void BinaryViewer::wheelEvent(QWheelEvent *e) {
    sb_->event(e);
    e->accept();
}
