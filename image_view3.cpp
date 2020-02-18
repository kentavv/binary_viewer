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

#include <QtGui>
#include <QGridLayout>
#include <QSpinBox>
#include <QComboBox>

#include "image_view3.h"
#include "bayer.h"


ImageView3::ImageView3(QWidget *p)
        : QLabel(p),
          dat_(NULL), dat_n_(0), inverted_(true) {
    {
        QGridLayout *layout = new QGridLayout(this);
        {
            QLabel *l = new QLabel("Offset (B)");
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, 0, 0);
        }
        {
            QSpinBox *sb = new QSpinBox;
            sb->setFixedSize(sb->sizeHint());
            sb->setFixedWidth(sb->width() * 1.5);
            sb->setRange(0, 100000);
            sb->setValue(0);
            offset_ = sb;
            layout->addWidget(sb, 0, 1);
        }
        {
            QLabel *l = new QLabel("Width");
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, 1, 0);
        }
        {
            QSpinBox *sb = new QSpinBox;
            sb->setFixedSize(sb->sizeHint());
            sb->setFixedWidth(sb->width() * 1.5);
            sb->setRange(1, 10000);
            sb->setValue(512);
            width_ = sb;
            layout->addWidget(sb, 1, 1);
        }
        {
            QLabel *l = new QLabel("Type");
            l->setFixedSize(l->sizeHint());
            layout->addWidget(l, 2, 0);
        }
        {
            QComboBox *cb = new QComboBox;
            cb->setFixedSize(cb->sizeHint());
            cb->addItem("RGB 8");
            cb->addItem("RGB 12");
            cb->addItem("RGB 16");
            cb->addItem("RGBA 8");
            cb->addItem("RGBA 12");
            cb->addItem("RGBA 16");
            cb->addItem("BGR 8");
            cb->addItem("BGR 12");
            cb->addItem("BGR 16");
            cb->addItem("BGRA 8");
            cb->addItem("BGRA 12");
            cb->addItem("BGRA 16");
            cb->addItem("Grey 8");
            cb->addItem("Grey 12");
            cb->addItem("Grey 16");
            cb->addItem("Bayer 8 - 0: 0 1 2 3");
            cb->addItem("Bayer 8 - 1: 0 1 3 2");
            cb->addItem("Bayer 8 - 2: 0 2 1 3");
            cb->addItem("Bayer 8 - 3: 0 2 3 1");
            cb->addItem("Bayer 8 - 4: 0 3 1 2");
            cb->addItem("Bayer 8 - 5: 0 3 2 1");
            cb->addItem("Bayer 8 - 6: 1 0 2 3");
            cb->addItem("Bayer 8 - 7: 1 0 3 2");
            cb->addItem("Bayer 8 - 8: 1 2 0 3");
            cb->addItem("Bayer 8 - 9: 1 2 3 0");
            cb->addItem("Bayer 8 - 10: 1 3 0 2");
            cb->addItem("Bayer 8 - 11: 1 3 2 0");
            cb->addItem("Bayer 8 - 12: 2 0 1 3");
            cb->addItem("Bayer 8 - 13: 2 0 3 1");
            cb->addItem("Bayer 8 - 14: 2 1 0 3");
            cb->addItem("Bayer 8 - 15: 2 1 3 0");
            cb->addItem("Bayer 8 - 16: 2 3 0 1");
            cb->addItem("Bayer 8 - 17: 2 3 1 0");
            cb->addItem("Bayer 8 - 18: 3 0 1 2");
            cb->addItem("Bayer 8 - 19: 3 0 2 1");
            cb->addItem("Bayer 8 - 20: 3 1 0 2");
            cb->addItem("Bayer 8 - 21: 3 1 2 0");
            cb->addItem("Bayer 8 - 22: 3 2 0 1");
            cb->addItem("Bayer 8 - 23: 3 2 1 0");
            cb->setCurrentIndex(0);
            cb->setEditable(false);
            cb->setFixedWidth(cb->width() * 1.5);
            type_ = cb;
            layout->addWidget(cb, 2, 1);
        }

        layout->setColumnStretch(2, 1);
        layout->setRowStretch(3, 1);

        QObject::connect(offset_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
        QObject::connect(width_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
        QObject::connect(type_, SIGNAL(currentIndexChanged(int)), this, SLOT(parameters_changed()));
    }
}

ImageView3::~ImageView3() {
}

void ImageView3::setImage(QImage &img) {
    img_ = img;

    update_pix();

    update();
}

void ImageView3::paintEvent(QPaintEvent *e) {
    QLabel::paintEvent(e);

    QPainter p(this);
    {
        // a border around the image helps to see the border of a dark image
        p.setPen(Qt::darkGray);
        p.drawRect(0, 0, width() - 1, height() - 1);
    }
}

void ImageView3::resizeEvent(QResizeEvent *e) {
    QLabel::resizeEvent(e);

    update_pix();
}

void ImageView3::update_pix() {
    //img_.scaled(size()).save("test.png", "png");
    pix_ = QPixmap::fromImage(img_.scaled(size())); //, Qt::KeepAspectRatio);
    //pix_ = QPixmap::fromImage(img_);
    setPixmap(pix_);
}


void ImageView3::setData(const unsigned char *dat, long n) {
    dat_ = dat;
    dat_n_ = n;

    regen_image();
}

void ImageView3::regen_image() {
    parameters_changed();
}

void ImageView3::parameters_changed() {
    int offset = offset_->value();
    int w = width_->value();

    dtype_t t;
    QString s = type_->currentText();
    if (s == "RGB 8") t = rgb8;
    else if (s == "RGB 12") t = rgb12;
    else if (s == "RGB 16") t = rgb16;
    else if (s == "RGBA 8") t = rgba8;
    else if (s == "RGBA 12") t = rgba12;
    else if (s == "RGBA 16") t = rgba16;
    else if (s == "BGR 8") t = bgr8;
    else if (s == "BGR 12") t = bgr12;
    else if (s == "BGR 16") t = bgr16;
    else if (s == "BGRA 8") t = bgra8;
    else if (s == "BGRA 12") t = bgra12;
    else if (s == "BGRA 16") t = bgra16;
    else if (s == "Grey 8") t = grey8;
    else if (s == "Grey 12") t = grey12;
    else if (s == "Grey 16") t = grey16;
    else if (s == "Bayer 8 - 0: 0 1 2 3") t = bayer8_0;
    else if (s == "Bayer 8 - 1: 0 1 3 2") t = bayer8_1;
    else if (s == "Bayer 8 - 2: 0 2 1 3") t = bayer8_2;
    else if (s == "Bayer 8 - 3: 0 2 3 1") t = bayer8_3;
    else if (s == "Bayer 8 - 4: 0 3 1 2") t = bayer8_4;
    else if (s == "Bayer 8 - 5: 0 3 2 1") t = bayer8_5;
    else if (s == "Bayer 8 - 6: 1 0 2 3") t = bayer8_6;
    else if (s == "Bayer 8 - 7: 1 0 3 2") t = bayer8_7;
    else if (s == "Bayer 8 - 8: 1 2 0 3") t = bayer8_8;
    else if (s == "Bayer 8 - 9: 1 2 3 0") t = bayer8_9;
    else if (s == "Bayer 8 - 10: 1 3 0 2") t = bayer8_10;
    else if (s == "Bayer 8 - 11: 1 3 2 0") t = bayer8_11;
    else if (s == "Bayer 8 - 12: 2 0 1 3") t = bayer8_12;
    else if (s == "Bayer 8 - 13: 2 0 3 1") t = bayer8_13;
    else if (s == "Bayer 8 - 14: 2 1 0 3") t = bayer8_14;
    else if (s == "Bayer 8 - 15: 2 1 3 0") t = bayer8_15;
    else if (s == "Bayer 8 - 16: 2 3 0 1") t = bayer8_16;
    else if (s == "Bayer 8 - 17: 2 3 1 0") t = bayer8_17;
    else if (s == "Bayer 8 - 18: 3 0 1 2") t = bayer8_18;
    else if (s == "Bayer 8 - 19: 3 0 2 1") t = bayer8_19;
    else if (s == "Bayer 8 - 20: 3 1 0 2") t = bayer8_20;
    else if (s == "Bayer 8 - 21: 3 1 2 0") t = bayer8_21;
    else if (s == "Bayer 8 - 22: 3 2 0 1") t = bayer8_22;
    else if (s == "Bayer 8 - 23: 3 2 1 0") t = bayer8_23;
    else t = none;

    QImage img;

    switch (t) {
        case rgb8: {
            const unsigned char *dat_u8 = (const unsigned char *) (dat_ + offset);
            int n = (dat_n_ - offset) / 1 / 3;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            unsigned int *p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = dat_u8[i * 3 + 0];
                unsigned char g = dat_u8[i * 3 + 1];
                unsigned char b = dat_u8[i * 3 + 2];
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case rgb12: {
            const unsigned short *dat_u16 = (const unsigned short *) (dat_ + offset);
            int n = (dat_n_ - offset) / 2 / 3;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            unsigned int *p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = (dat_u16[i * 3 + 0] >> 4) & 0xff;
                unsigned char g = (dat_u16[i * 3 + 1] >> 4) & 0xff;
                unsigned char b = (dat_u16[i * 3 + 2] >> 4) & 0xff;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case rgb16: {
            const unsigned short *dat_u16 = (const unsigned short *) (dat_ + offset);
            int n = (dat_n_ - offset) / 2 / 3;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            unsigned int *p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = (dat_u16[i * 3 + 0] >> 8) & 0xff;
                unsigned char g = (dat_u16[i * 3 + 1] >> 8) & 0xff;
                unsigned char b = (dat_u16[i * 3 + 2] >> 8) & 0xff;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case rgba8: {
            const unsigned char *dat_u8 = (const unsigned char *) (dat_ + offset);
            int n = (dat_n_ - offset) / 1 / 4;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            unsigned int *p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = dat_u8[i * 4 + 0];
                unsigned char g = dat_u8[i * 4 + 1];
                unsigned char b = dat_u8[i * 4 + 2];
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case rgba12: {
            const unsigned short *dat_u16 = (const unsigned short *) (dat_ + offset);
            int n = (dat_n_ - offset) / 2 / 4;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            unsigned int *p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = (dat_u16[i * 4 + 0] >> 4) & 0xff;
                unsigned char g = (dat_u16[i * 4 + 1] >> 4) & 0xff;
                unsigned char b = (dat_u16[i * 4 + 2] >> 4) & 0xff;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case rgba16: {
            const unsigned short *dat_u16 = (const unsigned short *) (dat_ + offset);
            int n = (dat_n_ - offset) / 2 / 4;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            unsigned int *p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = dat_u16[i * 4 + 0] >> 8;
                unsigned char g = dat_u16[i * 4 + 1] >> 8;
                unsigned char b = dat_u16[i * 4 + 2] >> 8;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case bgr8: {
            const unsigned char *dat_u8 = (const unsigned char *) (dat_ + offset);
            int n = (dat_n_ - offset) / 1 / 3;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            unsigned int *p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = dat_u8[i * 3 + 2];
                unsigned char g = dat_u8[i * 3 + 1];
                unsigned char b = dat_u8[i * 3 + 0];
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case bgr12: {
            const unsigned short *dat_u16 = (const unsigned short *) (dat_ + offset);
            int n = (dat_n_ - offset) / 2 / 3;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            unsigned int *p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = (dat_u16[i * 3 + 2] >> 4) & 0xff;
                unsigned char g = (dat_u16[i * 3 + 1] >> 4) & 0xff;
                unsigned char b = (dat_u16[i * 3 + 0] >> 4) & 0xff;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case bgr16: {
            const unsigned short *dat_u16 = (const unsigned short *) (dat_ + offset);
            int n = (dat_n_ - offset) / 2 / 3;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            unsigned int *p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = (dat_u16[i * 3 + 2] >> 8) & 0xff;
                unsigned char g = (dat_u16[i * 3 + 1] >> 8) & 0xff;
                unsigned char b = (dat_u16[i * 3 + 0] >> 8) & 0xff;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case bgra8: {
            const unsigned char *dat_u8 = (const unsigned char *) (dat_ + offset);
            int n = (dat_n_ - offset) / 1 / 4;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            unsigned int *p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = dat_u8[i * 4 + 2];
                unsigned char g = dat_u8[i * 4 + 1];
                unsigned char b = dat_u8[i * 4 + 0];
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case bgra12: {
            const unsigned short *dat_u16 = (const unsigned short *) (dat_ + offset);
            int n = (dat_n_ - offset) / 2 / 4;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            unsigned int *p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = (dat_u16[i * 4 + 2] >> 4) & 0xff;
                unsigned char g = (dat_u16[i * 4 + 1] >> 4) & 0xff;
                unsigned char b = (dat_u16[i * 4 + 0] >> 4) & 0xff;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case bgra16: {
            const unsigned short *dat_u16 = (const unsigned short *) (dat_ + offset);
            int n = (dat_n_ - offset) / 2 / 4;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            unsigned int *p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = (dat_u16[i * 4 + 2] >> 8) & 0xff;
                unsigned char g = (dat_u16[i * 4 + 1] >> 8) & 0xff;
                unsigned char b = (dat_u16[i * 4 + 0] >> 8) & 0xff;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case grey8: {
            const unsigned char *dat_u8 = (const unsigned char *) (dat_ + offset);
            int n = (dat_n_ - offset) / 1;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            unsigned int *p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char c = dat_u8[i];
                unsigned char r = c;
                unsigned char g = c;
                unsigned char b = c;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case grey12: {
            const unsigned short *dat_u16 = (const unsigned short *) (dat_ + offset);
            int n = (dat_n_ - offset) / 2;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            unsigned int *p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char c = (dat_u16[i] >> 4) & 0xff;
                unsigned char r = c;
                unsigned char g = c;
                unsigned char b = c;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case grey16: {
            const unsigned short *dat_u16 = (const unsigned short *) (dat_ + offset);
            int n = (dat_n_ - offset) / 2;
            img = QImage(w, n / w + 1, QImage::Format_RGB32);
            img.fill(0);
            unsigned int *p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char c = (dat_u16[i] >> 8) & 0xff;
                unsigned char r = c;
                unsigned char g = c;
                unsigned char b = c;
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        case bayer8_0:
        case bayer8_1:
        case bayer8_2:
        case bayer8_3:
        case bayer8_4:
        case bayer8_5:
        case bayer8_6:
        case bayer8_7:
        case bayer8_8:
        case bayer8_9:
        case bayer8_10:
        case bayer8_11:
        case bayer8_12:
        case bayer8_13:
        case bayer8_14:
        case bayer8_15:
        case bayer8_16:
        case bayer8_17:
        case bayer8_18:
        case bayer8_19:
        case bayer8_20:
        case bayer8_21:
        case bayer8_22:
        case bayer8_23: {
            int h = dat_n_ / w + 1;
            //int bayer_n = w * h;

            const unsigned char *dat_u8 = (const unsigned char *) (dat_ + offset);

            const unsigned char *bayer = dat_u8;
            unsigned char *rgb = new unsigned char[w * h * 3];
            int perm = 0;
            switch (t) {
                case bayer8_0:
                    perm = 0;
                    break;
                case bayer8_1:
                    perm = 1;
                    break;
                case bayer8_2:
                    perm = 2;
                    break;
                case bayer8_3:
                    perm = 3;
                    break;
                case bayer8_4:
                    perm = 4;
                    break;
                case bayer8_5:
                    perm = 5;
                    break;
                case bayer8_6:
                    perm = 6;
                    break;
                case bayer8_7:
                    perm = 7;
                    break;
                case bayer8_8:
                    perm = 8;
                    break;
                case bayer8_9:
                    perm = 9;
                    break;
                case bayer8_10:
                    perm = 10;
                    break;
                case bayer8_11:
                    perm = 11;
                    break;
                case bayer8_12:
                    perm = 12;
                    break;
                case bayer8_13:
                    perm = 13;
                    break;
                case bayer8_14:
                    perm = 14;
                    break;
                case bayer8_15:
                    perm = 15;
                    break;
                case bayer8_16:
                    perm = 16;
                    break;
                case bayer8_17:
                    perm = 17;
                    break;
                case bayer8_18:
                    perm = 18;
                    break;
                case bayer8_19:
                    perm = 19;
                    break;
                case bayer8_20:
                    perm = 20;
                    break;
                case bayer8_21:
                    perm = 21;
                    break;
                case bayer8_22:
                    perm = 22;
                    break;
                case bayer8_23:
                    perm = 23;
                    break;
            }
            bayerBG(bayer, h, w, perm, rgb);

            int n = (dat_n_ - offset) / 1;
            img = QImage(w, h, QImage::Format_RGB32);
            img.fill(0);
            unsigned int *p = (unsigned int *) img.bits();
            for (int i = 0; i < n; i++) {
                unsigned char r = rgb[i * 3 + 0];
                unsigned char g = rgb[i * 3 + 1];
                unsigned char b = rgb[i * 3 + 2];
                unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
                *p++ = v;
            }
        }
            break;
        default:
            abort();
    }

    if (inverted_) {
        img = img.mirrored(true);
    }
    setImage(img);
}
