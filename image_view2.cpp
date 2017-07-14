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

#include <cfloat>

#include <QtGui>

#include "image_view2.h"

ImageView2::ImageView2(QWidget *p)
  : QLabel(p),
    hist_(NULL), dat_(NULL), dat_n_(0)
{
  {
    QGridLayout *layout = new QGridLayout(this);
    {
      QLabel *l = new QLabel("Threshold");
      l->setFixedSize(l->sizeHint());
      layout->addWidget(l, 0, 0);
    }
    {
      QSpinBox *sb = new QSpinBox;
      sb->setFixedSize(sb->sizeHint());
      sb->setFixedWidth(sb->width() * 1.5);
      sb->setRange(1, 10000);
      sb->setValue(4);
      thresh_ = sb;
      layout->addWidget(sb, 0, 1);
    }
    {
      QLabel *l = new QLabel("Scale");
      l->setFixedSize(l->sizeHint());
      layout->addWidget(l, 1, 0);
    }
    {
      QSpinBox *sb = new QSpinBox;
      sb->setFixedSize(sb->sizeHint());
      sb->setFixedWidth(sb->width() * 1.5);
      sb->setRange(1, 10000);
      sb->setValue(100);
      scale_ = sb;
      layout->addWidget(sb, 1, 1);
    }
    {
      QLabel *l = new QLabel("Type");
      l->setFixedSize(l->sizeHint());
      layout->addWidget(l, 2, 0);
    }
    {
      QComboBox *cb = new QComboBox;
      cb->setFixedSize(cb->sizeHint());
      cb->addItem("U8");
      cb->addItem("U16");
      cb->addItem("U32");
      cb->addItem("U64");
      cb->addItem("F32");
      cb->addItem("F64");
      cb->setCurrentIndex(0);
      cb->setEditable(false);
      type_ = cb;
      layout->addWidget(cb, 2, 1);
    }

    layout->setColumnStretch(2, 1);
    layout->setRowStretch(3, 1);

    QObject::connect(thresh_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
    QObject::connect(scale_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
    QObject::connect(type_, SIGNAL(currentIndexChanged(int)), this, SLOT(regen_histo()));
  }
}

ImageView2::~ImageView2() {
  if(hist_) delete[] hist_;
}

void ImageView2::setImage(QImage &img) { 
  img_ = img;

  update_pix();

  update();
}

void ImageView2::paintEvent(QPaintEvent *e) {
  QLabel::paintEvent(e);

  QPainter p(this);
  {
    // a border around the image helps to see the border of a dark image
    p.setPen(Qt::darkGray);
    p.drawRect(0, 0, width()-1, height()-1);
  }
}

void ImageView2::resizeEvent(QResizeEvent *e) {
  QLabel::resizeEvent(e);

  update_pix();
}

void ImageView2::update_pix() {
  int vw = width();
  int vh = height();
  pix_ = QPixmap::fromImage(img_).scaled(vw, vh); //, Qt::KeepAspectRatio);
  setPixmap(pix_);
}

template<class T> void hist_float_helper_2d(int *hist, T *dat_f, long n) {
  for(long i=0; i<n/long(sizeof(T))-1; i++) {
    int a1;
    int a2;
    if(sizeof(T) == 4) {
      a1 = ((dat_f[i+0] / FLT_MAX) * 255. + 255.) / 2.;
      a2 = ((dat_f[i+1] / FLT_MAX) * 255. + 255.) / 2.;
    } else if(sizeof(T) == 8) {
      a1 = ((dat_f[i+0] / DBL_MAX) * 255. + 255.) / 2.;
      a2 = ((dat_f[i+1] / DBL_MAX) * 255. + 255.) / 2.;
    } else {
      abort();
    }
    
    if(isnan(dat_f[i+0])) a1 = 255;
    if(isnan(dat_f[i+1])) a2 = 255;
    
    if(isnan(dat_f[i+0])) { if(signbit(dat_f[i+0])) { a1 = 0; } else { a1 = 255; } }
    if(isnan(dat_f[i+1])) { if(signbit(dat_f[i+1])) { a2 = 0; } else { a2 = 255; } } 
    
    if(isinf(dat_f[i+0])) { if(signbit(dat_f[i+0])) { a1 = 0; } else { a1 = 255; } }
    if(isinf(dat_f[i+1])) { if(signbit(dat_f[i+1])) { a2 = 0; } else { a2 = 255; } } 
   
#if 0 
    if(a1 < 0 || a1 > 255) {
      printf("0 %d %f\n", a1, dat_f[i+0]);
    }
    if(a2 < 0 || a2 > 255) {
      printf("1 %d %f %d %d\n", a2, dat_f[i+1], isnan(dat_f[i+1]), dat_f[i+1] < 0);
    }
#endif

    if(a1 < 0) a1 = 0;
    if(a2 < 0) a2 = 0;
    
    if(a1 > 255) a1 = 255;
    if(a2 > 255) a2 = 255;
    
    hist[a1 * 256 + a2]++;
  }
}

int *ImageView2::generate_histo(const unsigned char *dat_u8, long n, dtype_t dtype) {
  int *hist = new int[256 * 256];
  memset(hist, 0, sizeof(hist[0]) * 256 * 256);

  switch(dtype) {
  case none:
    break;
  case u8:
    {
      for(long i=0; i<n-1; i++) {
        int a1 = dat_u8[i+0];
        int a2 = dat_u8[i+1];
        
        hist[a1 * 256 + a2]++;
      }
    }
    break;
  case u16:
    {
      const unsigned short *dat_u16 = (const unsigned short*)dat_u8;
      for(long i=0; i<n/2-1; i++) {
        int a1 = dat_u16[i+0] / float(0xffff) * 255.;
        int a2 = dat_u16[i+1] / float(0xffff) * 255.;
        
        hist[a1 * 256 + a2]++;
      }
    }
    break;
  case u32:
    {
      const unsigned int *dat_u32 = (const unsigned int*)dat_u8;
      for(long i=0; i<n/4-1; i++) {
        int a1 = dat_u32[i+0] / float(0xffffffff) * 255.;
        int a2 = dat_u32[i+1] / float(0xffffffff) * 255.;
    
        hist[a1 * 256 + a2]++;
      }
    }
    break;
  case u64:
    {
      const unsigned long *dat_u64 = (const unsigned long*)dat_u8;
      for(long i=0; i<n/8-1; i++) {
        int a1 = dat_u64[i+0] / float(0xffffffffffffffff) * 255.;
        int a2 = dat_u64[i+1] / float(0xffffffffffffffff) * 255.;
    
        hist[a1 * 256 + a2]++;
      }
    }
    break;
  case f32:
    {
      const float *dat_f32 = (const float*)dat_u8;
      hist_float_helper_2d(hist, dat_f32, n);
    }
  case f64:
    {
      const double *dat_f64 = (const double*)dat_u8;
      hist_float_helper_2d(hist, dat_f64, n);
    }    
    break;
  }
 
#if 0 
  int n_vertices = 0;
  float m=10000000, M=-1, a=0.;
  for(int i=0; i<256*256; i++) {
    if(hist[i] > 0) {
      n_vertices++;

      if(m > hist[i]) m = hist[i];
      if(M < hist[i]) M = hist[i];
      a += hist[i];
    }
  }
  a /= n_vertices;
  printf("%d %f %f %f\n", n_vertices, m, M, a);
#endif

  return hist;
}

void ImageView2::setData(const unsigned char *dat, long n) {
  dat_ = dat;
  dat_n_ = n;

  regen_histo();
}

void ImageView2::regen_histo() {
  if(hist_) {
    delete[] hist_;
    hist_ = NULL;
  }

  dtype_t t;
  QString s = type_->currentText();
  if(s == "U8") t = u8;
  else if(s == "U16") t = u16;
  else if(s == "U32") t = u32;
  else if(s == "U64") t = u64;
  else if(s == "F32") t = f32;
  else if(s == "F64") t = f64;
  else t = none;

  hist_ = generate_histo(dat_, dat_n_, t);

  parameters_changed();
}

void ImageView2::parameters_changed() {
  int thresh = thresh_->value();
  float scale_factor = scale_->value();
  
  QImage img(256, 256, QImage::Format_RGB32);
  img.fill(0);

  unsigned int *p = (unsigned int*)img.bits();
  
  for(int i=0; i<256*256; i++, p++) {
    if(hist_[i] >= thresh) {
      float cc = hist_[i] / scale_factor;
      cc += .2;
      if(cc > 1.) cc = 1.;
      int c = cc * 255 + .5;
      if(c < 0) c = 0;
      if(c > 255) c = 255;

      unsigned char r = 20;
      unsigned char g = c;
      unsigned char b = 20;
      unsigned int v = 0xff000000 | (r << 16) | (g << 8) | (b << 0);

      *p = v;
    }
  }

  setImage(img);
  
  update();
}
