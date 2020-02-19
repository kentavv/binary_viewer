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

#ifndef _GRAPH_VIEW_H_
#define _GRAPH_VIEW_H_

#include <QLabel>
#include <QImage>
#include <QPixmap>

class GraphView : public QLabel {
Q_OBJECT
public:
    explicit GraphView(QWidget *p = nullptr);

    ~GraphView() override;

public slots:

    void setImage(int ind, QImage &img);

    void set_data(const float *bin, long len, bool normalize = true);

    void set_data(int ind, const float *bin, long len, bool normalize = true);

    void enableSelection(bool);

protected slots:

protected:
    QImage img_[2];
    QPixmap pix_;

    void paintEvent(QPaintEvent *) override;

    void resizeEvent(QResizeEvent *e) override;

    void mousePressEvent(QMouseEvent *event) override;

    void mouseMoveEvent(QMouseEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;

    void update_pix();

    float m1_, m2_;
    int px_, py_;
    int ind_;
    enum {
        none, m1_moving, m2_moving, m12_moving
    } s_;
    bool allow_selection_;

signals:

    void rangeSelected(float, float);
};

#endif
