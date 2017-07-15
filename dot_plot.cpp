/*
 * MIT License
 * 
 * Copyright (c) 2015 Kent VanderVelden
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

#include <vector>
#include <algorithm>

#include <QtGui>
#include <QGridLayout>
#include <QSpinBox>
#include <QComboBox>

#include "dot_plot.h"

using std::min;
using std::vector;
using std::random_shuffle;
using std::pair;
using std::make_pair;

vector<pair<int, int> > pts;
int pts_i = 0;

DotPlot::DotPlot(QWidget *p)
  : QLabel(p),
    dat_(NULL), dat_n_(0)
{
  {
    QGridLayout *layout = new QGridLayout(this);
    int r = 0;
    {
      QLabel *l = new QLabel("Offset1 (B)");
      l->setFixedSize(l->sizeHint());
      layout->addWidget(l, r, 0);
    }
    {
      QSpinBox *sb = new QSpinBox;
      sb->setFixedSize(sb->sizeHint());
      sb->setFixedWidth(sb->width() * 1.5);
      sb->setRange(0, 100000);
      sb->setValue(0);
      offset1_ = sb;
      layout->addWidget(sb, r, 1);
    }
    r++;
    {
      QLabel *l = new QLabel("Offset2 (B)");
      l->setFixedSize(l->sizeHint());
      layout->addWidget(l, r, 0);
    }
    {
      QSpinBox *sb = new QSpinBox;
      sb->setFixedSize(sb->sizeHint());
      sb->setFixedWidth(sb->width() * 1.5);
      sb->setRange(0, 100000);
      sb->setValue(0);
      offset2_ = sb;
      layout->addWidget(sb, r, 1);
    }
    r++;
    {
      QLabel *l = new QLabel("Width");
      l->setFixedSize(l->sizeHint());
      layout->addWidget(l, r, 0);
    }
    {
      QSpinBox *sb = new QSpinBox;
      sb->setFixedSize(sb->sizeHint());
      sb->setFixedWidth(sb->width() * 1.5);
      sb->setRange(1, 100000);
      sb->setValue(10000);
      width_ = sb;
      layout->addWidget(sb, r, 1);
    }
    r++;
    {
      QLabel *l = new QLabel("Max Samples");
      l->setFixedSize(l->sizeHint());
      layout->addWidget(l, r, 0);
    }
    {
      QSpinBox *sb = new QSpinBox;
      sb->setFixedSize(sb->sizeHint());
      sb->setFixedWidth(sb->width() * 1.5);
      sb->setRange(1, 100000);
      sb->setValue(10);
      max_samples_ = sb;
      layout->addWidget(sb, r, 1);
    }
    r++;
    
    layout->setColumnStretch(2, 1);
    layout->setRowStretch(r, 1);

    QObject::connect(offset1_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
    QObject::connect(offset2_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
    QObject::connect(width_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
    QObject::connect(max_samples_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
  }
}

DotPlot::~DotPlot() {
}

void DotPlot::setImage(QImage &img) { 
  img_ = img;

  update_pix();

  update();
}

void DotPlot::paintEvent(QPaintEvent *e) {
  QLabel::paintEvent(e);

  QPainter p(this);
  {
    // a border around the image helps to see the border of a dark image
    p.setPen(Qt::darkGray);
    p.drawRect(0, 0, width()-1, height()-1);
  }
}

void DotPlot::resizeEvent(QResizeEvent *e) {
  QLabel::resizeEvent(e);

  parameters_changed();
}

void DotPlot::update_pix() {
  pix_ = QPixmap::fromImage(img_.scaled(size())); //, Qt::KeepAspectRatio);
  setPixmap(pix_);
}


void DotPlot::setData(const unsigned char *dat, long n) {
  dat_ = dat;
  dat_n_ = n;

  offset1_->setRange(0, dat_n_);
  offset2_->setRange(0, dat_n_);
  width_->setRange(1, dat_n_);

  width_->setValue(dat_n_);

  parameters_changed();

  for(int i=0; i < pts_i; i++) {
    advance_mat();
  }
  regen_image();
}

int *mat = NULL;

void DotPlot::parameters_changed() {
  if(mat) {
    delete[] mat;
    mat = NULL;
  }

  int mwh = min(width(), height());

  long mdw = min(dat_n_, (long)width_->value());
  
  if(mwh > mdw) mwh = mdw;

  if(mat == NULL) {
    mat = new int[mwh * mwh];
    memset(mat, 0, sizeof(mat[0]) * mwh * mwh);
  }

  pts.clear();
  for(int i=0; i<mwh; i++) {
    for(int j=i; j<mwh; j++) {
      pts.push_back(make_pair(i, j));
    }
  }
  pts_i = pts.size();
  random_shuffle(pts.begin(), pts.end());

  // pts_i is decremented in advance_mat()
  while(pts_i > 0) {
    advance_mat();
  }
  regen_image();
}

void DotPlot::advance_mat() {
  if(pts_i == 0 || pts.empty()) return;

  pts_i--;
  pair<int, int> pt = pts[pts_i];

  int mwh = min(width(), height());
  float sf = 1.;
  {
    long mdw = min(dat_n_, (long)width_->value());
    if(mwh > mdw) mwh = mdw;
    if(mwh < mdw) {
      sf = mwh / float(mdw);
    }
  }

  int x = pt.first;
  int y = pt.second;

  int xo = x/sf;
  int yo = y/sf;
  int xyn = 1/sf;

  for(int tt=0; tt<min(max_samples_->value(), xyn*xyn); tt++) {
    int i = random() % xyn + xo;
    int j = random() % xyn + yo;

    {
      if(dat_[i] == dat_[j]) {
        int ii = y * mwh + x;
        if(ii < 0 || mwh*mwh <= ii) {
          continue;
        }
        mat[ii]++;

        int jj = x * mwh + y;
        if(jj < 0 || mwh*mwh <= jj) {
          continue;
        }
        mat[jj]++;
      }
    }
  }
}

void DotPlot::regen_image() {
  int mwh = min(width(), height());
  long mdw = min(dat_n_, (long)width_->value());
  
  if(mwh > mdw) mwh = mdw;

  int m=0;
  for(int i=0; i<mwh*mwh; i++) {
    if(m < mat[i]) m = mat[i];
  }

  QImage img(mwh, mwh, QImage::Format_RGB32);
  img.fill(0);
  unsigned int *p = (unsigned int*)img.bits();
  for(int i=0; i<mwh*mwh; i++) {
    unsigned char c = mat[i] / float(m) * 255.;
    unsigned char r = c;
    unsigned char g = c;
    unsigned char b = c;
    unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);
    *p++ = v;
  }

  setImage(img);
}
