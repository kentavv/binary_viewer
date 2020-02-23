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
    explicit ImageView3(QWidget *p = nullptr);

    ~ImageView3() override = default;

public slots:

    void setData(const unsigned char *dat, long n);

    void parameters_changed();

protected slots:

    void setImage(QImage &img);

    void regen_image();

protected:
    QImage img_;
    QPixmap pix_;

    void paintEvent(QPaintEvent *) override;

    void resizeEvent(QResizeEvent *e) override;

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
