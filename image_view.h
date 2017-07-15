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

#ifndef _IMAGE_VIEW_H_
#define _IMAGE_VIEW_H_

#include <QLabel>
#include <QImage>
#include <QPixmap>

class ImageView : public QLabel {
  Q_OBJECT
public:
  ImageView(QWidget *p=NULL);
  ~ImageView();

public slots:
  void setImage(QImage &img);
  void set_data(const unsigned char *bin, long len);
  void enableSelection(bool);
  
protected slots:

protected:
  QImage img_;
  QPixmap pix_;

  void paintEvent(QPaintEvent *);
  void resizeEvent(QResizeEvent *e);
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);

  void update_pix();

  float m1_, m2_;
  int px_, py_;
  enum {none, m1_moving, m2_moving, m12_moving} s_;
  bool allow_selection_;
  
signals:
  void rangeSelected(float, float);
};

#endif
