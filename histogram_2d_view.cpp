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

#include <QtGui>
#include <QGridLayout>
#include <QSpinBox>
#include <QComboBox>

#include "histogram_2d_view.h"
#include "histogram_calc.h"

using std::isnan;
using std::signbit;
using std::isinf;


Histogram2dView::Histogram2dView(QWidget *p)
        : QLabel(p),
          hist_(nullptr), dat_(nullptr), dat_n_(0) {
    {
        auto layout = new QGridLayout(this);
        {
            auto l = new QLabel("Threshold");
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, 0, 0);
        }
        {
            auto sb = new QSpinBox;
            sb->setFixedSize(sb->sizeHint());
            sb->setFixedWidth(sb->width() * 1.5);
            sb->setRange(1, 10000);
            sb->setValue(4);
            thresh_ = sb;
            layout->addWidget(sb, 0, 1);
        }
        {
            auto l = new QLabel("Scale");
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, 1, 0);
        }
        {
            auto sb = new QSpinBox;
            sb->setFixedSize(sb->sizeHint());
            sb->setFixedWidth(sb->width() * 1.5);
            sb->setRange(1, 10000);
            sb->setValue(100);
            scale_ = sb;
            layout->addWidget(sb, 1, 1);
        }
        {
            auto l = new QLabel("Type");
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, 2, 0);
        }
        {
            auto cb = new QComboBox;
            cb->setFixedSize(cb->sizeHint());
            cb->addItem("U8");
            cb->addItem("U16");
            cb->addItem("U32");
            cb->addItem("U64");
            cb->addItem("F32");
            cb->addItem("F64");
            cb->setCurrentIndex(0);
            cb->setEditable(false);
            type_ = cb;
            layout->addWidget(cb, 2, 1);
        }

        layout->setColumnStretch(2, 1);
        layout->setRowStretch(3, 1);

        QObject::connect(thresh_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
        QObject::connect(scale_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
        QObject::connect(type_, SIGNAL(currentIndexChanged(int)), this, SLOT(regen_histo()));
    }
}

Histogram2dView::~Histogram2dView() {
    delete[] hist_;
}

void Histogram2dView::setImage(QImage &img) {
    img_ = img;

    update_pix();

    update();
}

void Histogram2dView::paintEvent(QPaintEvent *e) {
    QLabel::paintEvent(e);

    QPainter p(this);
    {
        // a border around the image helps to see the border of a dark image
        p.setPen(Qt::darkGray);
        p.drawRect(0, 0, width() - 1, height() - 1);
    }
}

void Histogram2dView::resizeEvent(QResizeEvent *e) {
    QLabel::resizeEvent(e);

    update_pix();
}

void Histogram2dView::update_pix() {
    if (img_.isNull()) return;

    int vw = width() - 4;
    int vh = height() - 4; // TODO BUG: With QDarkStyle, without the subtraction, the height or width of the application grows without bounds.
    pix_ = QPixmap::fromImage(img_).scaled(vw, vh); //, Qt::KeepAspectRatio);
    setPixmap(pix_);
}

void Histogram2dView::setData(const unsigned char *dat, long n) {
    dat_ = dat;
    dat_n_ = n;

    regen_histo();
}

void Histogram2dView::regen_histo() {
    delete[] hist_;
    hist_ = nullptr;

    histo_dtype_t t = string_to_histo_dtype(type_->currentText().toStdString());
    hist_ = generate_histo_2d(dat_, dat_n_, t);

    parameters_changed();
}

void Histogram2dView::parameters_changed() {
    int thresh = thresh_->value();
    float scale_factor = scale_->value();

    QImage img(256, 256, QImage::Format_RGB32);
    img.fill(0);

    auto p = (unsigned int *) img.bits();

    for (int i = 0; i < 256 * 256; i++, p++) {
        if (hist_[i] >= thresh) {
            float cc = hist_[i] / scale_factor;
            cc += .2;
            if (cc > 1.) cc = 1.;
            int c = cc * 255 + .5;
            if (c < 0) c = 0;
            if (c > 255) c = 255;

            unsigned char r = 20;
            unsigned char g = c;
            unsigned char b = 20;
            unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);

            *p = v;
        }
    }

    setImage(img);

    update();
}
