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

#ifndef _IMAGE_VIEW3_H_
#define _IMAGE_VIEW3_H_

#include <QLabel>
#include <QImage>
#include <QPixmap>

class QSpinBox;

class QComboBox;

class ImageView3 : public QLabel {
Q_OBJECT
public:
    ImageView3(QWidget *p = NULL);

    ~ImageView3();

public slots:

    void setData(const unsigned char *dat, long n);

    void parameters_changed();

protected slots:

    void setImage(QImage &img);

    void regen_image();

protected:
    QImage img_;
    QPixmap pix_;

    void paintEvent(QPaintEvent *);

    void resizeEvent(QResizeEvent *e);

    void update_pix();

    typedef enum {
        none, rgb8, rgb12, rgb16, rgba8, rgba12, rgba16, bgr8, bgr12, bgr16, bgra8, bgra12, bgra16, grey8, grey12, grey16,
        bayer8_0,
        bayer8_1,
        bayer8_2,
        bayer8_3,
        bayer8_4,
        bayer8_5,
        bayer8_6,
        bayer8_7,
        bayer8_8,
        bayer8_9,
        bayer8_10,
        bayer8_11,
        bayer8_12,
        bayer8_13,
        bayer8_14,
        bayer8_15,
        bayer8_16,
        bayer8_17,
        bayer8_18,
        bayer8_19,
        bayer8_20,
        bayer8_21,
        bayer8_22,
        bayer8_23
    } dtype_t;

    QSpinBox *offset_, *width_;
    QComboBox *type_;
    const unsigned char *dat_;
    long dat_n_;
    bool inverted_;
};

#endif
