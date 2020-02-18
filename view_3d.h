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

#ifndef _VIEW_3D_H_
#define _VIEW_3D_H_

#include <QGLWidget>

class QSpinBox;

class QComboBox;

class QCheckBox;

class View3D : public QGLWidget {
Q_OBJECT
public:
    View3D(QWidget *p = NULL);

    ~View3D();

public slots:

    void setData(const unsigned char *dat, long n);

    void parameters_changed();

protected slots:

    void regen_histo();

protected:
    void initializeGL();

    void resizeGL(int w, int h);

    void paintGL();

    void mousePressEvent(QMouseEvent *event);

    void mouseMoveEvent(QMouseEvent *event);

    void mouseReleaseEvent(QMouseEvent *event);

    typedef enum {
        none, u8, u12, u16, u32, u64, f32, f64
    } dtype_t;

    int *generate_histo(const unsigned char *, long int, dtype_t);

    QSpinBox *thresh_, *scale_;
    QComboBox *type_;
    QCheckBox *overlap_;
    int *hist_;
    const unsigned char *dat_;
    long dat_n_;
    bool spinning_;
};

#endif
