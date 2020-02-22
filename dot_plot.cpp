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

#include <vector>
#include <algorithm>

#include <QtGui>
#include <QGridLayout>
#include <QSpinBox>
#include <QComboBox>

#include "dot_plot.h"

using std::max;
using std::min;
using std::vector;
using std::random_shuffle;
using std::pair;
using std::make_pair;

DotPlot::DotPlot(QWidget *p)
        : QLabel(p),
          dat_(nullptr), dat_n_(0),
          mat_(nullptr),
          pts_i_(0) {
    {
        auto layout = new QGridLayout(this);
        int r = 0;

        {
            auto l = new QLabel("Offset1 (B)");
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, r, 0);
        }
        {
            auto sb = new QSpinBox;
            sb->setFixedSize(sb->sizeHint());
            sb->setFixedWidth(sb->width() * 1.5);
            sb->setRange(0, 100000);
            sb->setValue(0);
            offset1_ = sb;
            layout->addWidget(sb, r, 1);
        }
        r++;

        {
            auto l = new QLabel("Offset2 (B)");
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, r, 0);
        }
        {
            auto sb = new QSpinBox;
            sb->setFixedSize(sb->sizeHint());
            sb->setFixedWidth(sb->width() * 1.5);
            sb->setRange(0, 100000);
            sb->setValue(0);
            offset2_ = sb;
            layout->addWidget(sb, r, 1);
        }
        r++;

        {
            auto l = new QLabel("Width");
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, r, 0);
        }
        {
            auto sb = new QSpinBox;
            sb->setFixedSize(sb->sizeHint());
            sb->setFixedWidth(sb->width() * 1.5);
            sb->setRange(1, 100000);
            sb->setValue(10000);
            width_ = sb;
            layout->addWidget(sb, r, 1);
        }
        r++;

        {
            auto l = new QLabel("Max Samples");
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, r, 0);
        }
        {
            auto sb = new QSpinBox;
            sb->setFixedSize(sb->sizeHint());
            sb->setFixedWidth(sb->width() * 1.5);
            sb->setRange(1, 100000);
            sb->setValue(10);
            max_samples_ = sb;
            layout->addWidget(sb, r, 1);
        }
        r++;

        layout->setColumnStretch(2, 1);
        layout->setRowStretch(r, 1);

        QObject::connect(offset1_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
        QObject::connect(offset2_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
        QObject::connect(width_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
        QObject::connect(max_samples_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
    }
}

void DotPlot::setImage(QImage &img) {
    img_ = img;

    update_pix();

    update();
}

void DotPlot::paintEvent(QPaintEvent *e) {
    QLabel::paintEvent(e);

    QPainter p(this);
    {
        // a border around the image helps to see the border of a dark image
        p.setPen(Qt::darkGray);
        p.drawRect(0, 0, width() - 1, height() - 1);
    }
}

void DotPlot::resizeEvent(QResizeEvent *e) {
    QLabel::resizeEvent(e);

    parameters_changed();
}

void DotPlot::update_pix() {
    if (img_.isNull()) return;

    pix_ = QPixmap::fromImage(img_.scaled(size())); //, Qt::KeepAspectRatio);
    setPixmap(pix_);
}


void DotPlot::setData(const unsigned char *dat, long n) {
    dat_ = dat;
    dat_n_ = n;

    offset1_->setRange(0, dat_n_);
    offset2_->setRange(0, dat_n_);
    width_->setRange(1, dat_n_);

    width_->setValue(dat_n_);

    // parameters_changed() triggered by the previous setValue() call.
    // parameters_changed();
}

void DotPlot::parameters_changed() {
    puts("called");
    delete[] mat_;
    mat_ = nullptr;

    long mdw = min(dat_n_, (long) width_->value());
    int mwh = min(min(width(), height()), (int) mdw);

    // Could avoid re-allocation if size has not changed.
    mat_ = new int[mwh * mwh];
    memset(mat_, 0, sizeof(mat_[0]) * mwh * mwh);

    pts_.clear();
    pts_.reserve(mwh * mwh);
#if 1
    for (int i = 0; i < mwh; i++) {
        for (int j = i; j < mwh; j++) {
            pts_.emplace_back(make_pair(i, j));
        }
    }
#else
    for (int i = 0; i < mwh; i++) {
        pts_.emplace_back(make_pair(i, mwh-i-1));
        pts_.emplace_back(make_pair(i, i));
    }
#endif
    random_shuffle(pts_.begin(), pts_.end());

    {
        float sf = 1.f;
        if (mwh < mdw) {
            sf = mwh / float(mdw);
        }

        int xyn = 1 / sf + 1;

        printf("min(width(), height()): %d mwh:%d mdw:%d dat_n:%d sf:%f xyn:%d", min(width(), height()), mwh, mdw, dat_n_, sf, xyn);

        // Precompute some random values for sampling
        std::vector<int> rand;
        {
            int n = min(max_samples_->value(), xyn * xyn);
            rand.reserve(n);
            printf("Generating %d values in range [0, %d-1]\n", n, xyn);
            for (int tt = 0; tt < n; tt++) {
                rand.emplace_back(random() % xyn);
            }
        }

        printf("pts_.size(): %d sf: %f\n", pts_.size(), sf);
        // pts_i_ is decremented in advance_mat()
        pts_i_ = pts_.size();
        while (pts_i_ > 0) {
//            random_shuffle(rand.begin(), rand.end());
            advance_mat(mwh, sf, rand);
        }
    }
    regen_image();
}

void DotPlot::advance_mat(int mwh, float sf, const vector<int> &rand) {
    if (pts_i_ == 0 || pts_.empty()) return;

    pts_i_--;
    pair<int, int> pt = pts_[pts_i_];

    int x = pt.first;
    int y = pt.second;

    int xo = x / sf;
    int yo = y / sf;

    for (int tt = 0; tt < rand.size(); tt++) {
        int i = xo + rand[tt];
        int j = yo + rand[tt];

        if (dat_[i] == dat_[j]) {
            int ii = y * mwh + x;
            int jj = x * mwh + y;
            if (0 <= ii && ii < mwh * mwh) mat_[ii]++;
            if (0 <= jj && jj < mwh * mwh) mat_[jj]++;
        }
    }
}

void DotPlot::regen_image() {
    int mwh = min(width(), height());
    long mdw = min(dat_n_, (long) width_->value());

    if (mwh > mdw) mwh = mdw;

    // Find the maximum value, ignoring the diagonal.
    // Could stop the search once m = max_samples_->value()
    int m = 0;
    for (int j = 0; j < mwh; j++) {
        for (int i = 0; i < j; i++) {
            int k = j * mwh + i;
            if (m < mat_[k]) m = mat_[k];
        }
        for (int i = j + 1; i < mwh; i++) {
            int k = j * mwh + i;
            if (m < mat_[k]) m = mat_[k];
        }
    }

    // Brighten up the image
    printf("max: %d\n", m);
    m = max(1, int(m * .75));
    printf("max: %d\n", m);

    QImage img(mwh, mwh, QImage::Format_RGB32);
    img.fill(0);
    auto p = (unsigned int *) img.bits();
    for (int i = 0; i < mwh * mwh; i++) {
        int c = min(255, int(mat_[i] / float(m) * 255. + .5));
        unsigned char r = c;
        unsigned char g = c;
        unsigned char b = c;
        unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
        *p++ = v;
    }

    setImage(img);
}
