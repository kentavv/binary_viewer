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

#ifndef _DOTPLOT_H_
#define _DOTPLOT_H_

#include <QLabel>
#include <QImage>
#include <QPixmap>

class QSpinBox;

class DotPlot : public QLabel {
Q_OBJECT
public:
    DotPlot(QWidget *p = NULL);

    ~DotPlot();

public slots:

    void setData(const unsigned char *dat, long n);

    void parameters_changed();

protected slots:

    void setImage(QImage &img);

    void advance_mat();

    void regen_image();

protected:
    QImage img_;
    QPixmap pix_;

    void paintEvent(QPaintEvent *);

    void resizeEvent(QResizeEvent *e);

    void update_pix();

    typedef enum {
        none, rgb8, rgb16, rgba8, rgba16, bgr8, bgr16, bgra8, bgra16, grey8, grey16
    } dtype_t;

    QSpinBox *offset1_, *offset2_, *width_, *max_samples_;
    const unsigned char *dat_;
    long dat_n_;
};

#endif
