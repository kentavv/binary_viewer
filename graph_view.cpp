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

#include <algorithm>
#include <QtGui>

#include "graph_view.h"

using std::min;
using std::max;

GraphView::GraphView(QWidget *p)
        : QLabel(p),
          m1_(0.), m2_(1.), ind_(0), s_(none), allow_selection_(true) {
}

GraphView::~GraphView() {
}

void GraphView::enableSelection(bool v) {
  allow_selection_ = v;
  update();
}

void GraphView::setImage(int ind, QImage &img) {
  img_[ind] = img;

  update_pix();

  update();
}

void GraphView::set_data(const float *dat, long len, bool normalize) {
  ind_ = 0;
  set_data(0, dat, len, normalize);
}

void GraphView::set_data(int ind, const float *dat, long len, bool normalize) {
  int w = width();
  int h = height();

  float mn = 0.;
  float mx = 1.;
  if (normalize) {
    mn = 99999999.;
    mx = -99999999.;
    for (int i = 0; i < len; i++) {
      mn = min(mn, dat[i]);
      mx = max(mx, dat[i]);
    }
    if (mn == mx) {
      mn -= .5;
      mx += .5;
    }
  }

  {
    QImage img(w, h, QImage::Format_RGB32);
    img.fill(0);

    float *acc = new float[h];
    memset(acc, 0, h * sizeof(float));
    int *cnt = new int[h];
    memset(cnt, 0, h * sizeof(int));

    for (int i = 0; i < len; i++) {
      float v = dat[i];
      int ind = int((i / float(len)) * (h - 1) + .5);
      acc[ind] += (v - mn) / (mx - mn);
      cnt[ind]++;
    }

    unsigned int *p = (unsigned int *) img.bits();
    int px = -1;
    int pc = -1;
    for (int i = 0; i < h; i++) {
      int x = px;
      int c = pc;
      if (cnt[i] == 0 && px == -1) continue;
      if (cnt[i] > 0) {
        float na = acc[i] / cnt[i];
        x = int(na * (w - 4) + .5) + 2; // slight offset so not to interfere with border
        px = x;
        c = 20 + int(na * (255 - 20));
        pc = c;
      }
      unsigned char r = 20;
      unsigned char g = min(c + 60, 255);
      unsigned char b = 20;
      unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
      p[i * w + x] = v;
    }

    delete[] acc;
    delete[] cnt;

    setImage(ind, img);
  }
}

void GraphView::paintEvent(QPaintEvent *e) {
  QLabel::paintEvent(e);

  QPainter p(this);
  if (allow_selection_) {
    int ry1 = m1_ * height();
    int ry2 = m2_ * height();

    QBrush brush(QColor(128, 64, 64, 128 + 32));
    QPen pen(brush, 5, Qt::SolidLine, Qt::RoundCap);
    p.setPen(pen);
    p.drawLine(0 + 3, ry1, width() - 1 - 3, ry1);
    p.drawLine(0 + 3, ry2, width() - 1 - 3, ry2);
  }

  {
    // a border around the image helps to see the border of a dark image
    p.setPen(Qt::darkGray);
    p.drawRect(0, 0, width() - 1, height() - 1);
  }
}

void GraphView::resizeEvent(QResizeEvent *e) {
  QLabel::resizeEvent(e);

  update_pix();
}

void GraphView::update_pix() {
  int vw = width();
  int vh = height();
  pix_ = QPixmap::fromImage(img_[ind_]).scaled(vw, vh); //, Qt::KeepAspectRatio);
  setPixmap(pix_);
}

void GraphView::mousePressEvent(QMouseEvent *e) {
  e->accept();

  if (!allow_selection_) return;
  if (e->button() != Qt::LeftButton) return;

  int x = e->pos().x();
  int y = e->pos().y();

  if (x < 0) x = 0;
  if (x > width() - 1) x = width() - 1;
  if (y < 0) y = 0;
  if (y > height() - 1) y = height() - 1;

  float yp = y / float(height());

  if (yp > m1_ && (yp - m1_) < .01) {
    s_ = m1_moving;
  } else if (yp < m2_ && (m2_ - yp) < .01) {
    s_ = m2_moving;
  } else if (m1_ < yp && yp < m2_) {
    s_ = m12_moving;
  } else {
    s_ = none;
  }

  px_ = x;
  py_ = y;
}

void GraphView::mouseMoveEvent(QMouseEvent *e) {
  e->accept();

  if (s_ == none) return;

  int x = e->pos().x();
  int y = e->pos().y();

  if (x < 0) x = 0;
  if (x > width() - 1) x = width() - 1;
  if (y < 0) y = 0;
  if (y > height() - 1) y = height() - 1;

  if (y == py_) return;

  int h = height();

  float m1 = m1_;
  float m2 = m2_;
  if (s_ == m1_moving) {
    m1 = y / float(h);
  } else if (s_ == m2_moving) {
    m2 = y / float(h);
  } else if (s_ == m12_moving) {
    float dy = (y - py_) / float(h);
    m1 += dy;
    m2 += dy;
  }
  if (m1 >= m2_ - .01) m1 = m2_ - .01;
  if (m1 < 0.) m1 = 0.;
  if (m2 <= m1_ + .01) m2 = m1_ + .01;
  if (m2 > 1.) m2 = 1.;

  m1_ = m1;
  m2_ = m2;

  px_ = x;
  py_ = y;

  update();

  emit(rangeSelected(m1_, m2_));
}

void GraphView::mouseReleaseEvent(QMouseEvent *e) {
  e->accept();

  if (e->button() == Qt::RightButton) {
    ind_ = (ind_ + 1) % 2;
    update_pix();
    update();
  }

  if (e->button() != Qt::LeftButton) return;

  int x = e->pos().x();
  int y = e->pos().y();

  if (x < 0) x = 0;
  if (x > width() - 1) x = width() - 1;
  if (y < 0) y = 0;
  if (y > height() - 1) y = height() - 1;

  px_ = -1;
  py_ = -1;
  s_ = none;
}
