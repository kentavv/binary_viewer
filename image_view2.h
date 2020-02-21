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

#ifndef _IMAGE_VIEW2_H_
#define _IMAGE_VIEW2_H_

#include <QLabel>
#include <QImage>
#include <QPixmap>

class QSpinBox;

class QComboBox;

class ImageView2 : public QLabel {
Q_OBJECT
public:
    explicit ImageView2(QWidget *p = nullptr);

    ~ImageView2() override;

public slots:

    void setData(const unsigned char *dat, long n);

    void parameters_changed();

protected slots:

    void setImage(QImage &img);

    void regen_histo();

protected:
    QImage img_;
    QPixmap pix_;

    void paintEvent(QPaintEvent *) override;

    void resizeEvent(QResizeEvent *e) override;

    void update_pix();

    QSpinBox *thresh_, *scale_;
    QComboBox *type_;
    int *hist_;
    const unsigned char *dat_;
    long dat_n_;

signals:

    void rangeSelected(float, float);
};

#endif
