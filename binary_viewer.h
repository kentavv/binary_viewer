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

#ifndef _BINARY_VIEWER_
#define _BINARY_VIEWER_

#include <QWidget>

class BinaryView;

class QScrollBar;

class BinaryView : public QWidget {
Q_OBJECT
public:
    explicit BinaryView(QWidget *p = nullptr);

    ~BinaryView() override = default;

    int rowHeight() const;

public slots:

    void setData(const unsigned char *dat, long n);
    void setStart(int);

protected slots:

protected:
    QImage img_;
    QPixmap pix_;

    void paintEvent(QPaintEvent *) override;

    void resizeEvent(QResizeEvent *) override;

    const unsigned char *dat_;
    long dat_n_;
    int off_;
};

class BinaryViewer : public QWidget {
Q_OBJECT
public:
    explicit BinaryViewer(QWidget *p = nullptr);

    ~BinaryViewer() override;

public slots:

    void setData(const unsigned char *dat, long n);
    void setStart(int);

protected slots:

protected:
    QImage img_;
    QPixmap pix_;

    void paintEvent(QPaintEvent *) override;

    void resizeEvent(QResizeEvent *) override;

    void enterEvent(QEvent *) override;

    void wheelEvent(QWheelEvent *) override;

    BinaryView *bv_;
    QScrollBar *sb_;

    const unsigned char *dat_;
    long dat_n_;
};

#endif
