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
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>

#include <GL/glut.h>

#include "view_3d.h"

using std::isnan;
using std::signbit;
using std::isinf;


View3D::View3D(QWidget *p)
  : QGLWidget(p), hist_(NULL), dat_(NULL), dat_n_(0)
{
  QTimer *update_timer = new QTimer(this);
  QObject::connect(update_timer, SIGNAL(timeout()), this, SLOT(updateGL())); //, Qt::QueuedConnection);
  update_timer->start(100);

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
      cb->addItem("U12");
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
    {
      QLabel *l = new QLabel("Overlap");
      l->setFixedSize(l->sizeHint());
      layout->addWidget(l, 3, 0);
    }
    {
      QCheckBox *cb = new QCheckBox;
      cb->setChecked(true);
      overlap_ = cb;
      layout->addWidget(cb, 3, 1);
    }

    layout->setColumnStretch(2, 1);
    layout->setRowStretch(4, 1);

    QObject::connect(thresh_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
    QObject::connect(scale_, SIGNAL(valueChanged(int)), this, SLOT(parameters_changed()));
    QObject::connect(type_, SIGNAL(currentIndexChanged(int)), this, SLOT(regen_histo()));
    QObject::connect(overlap_, SIGNAL(toggled(bool)), this, SLOT(regen_histo()));
  }
}

View3D::~View3D() {
  if(hist_) delete[] hist_;
}

static float alpha = 0;
static float alpha2 = 0;

static GLfloat *vertices = NULL;
static GLfloat *colors = NULL;
int n_vertices = 0;

template<class T> void hist_float_helper_3d(int *hist, T *dat_f, long n, int st) {
  for(long i=0; i<n/long(sizeof(T))-2; i+=st) {
    int a1;
    int a2;
    int a3;
    if(sizeof(T) == 4) {
      a1 = ((dat_f[i+0] / FLT_MAX) * 255. + 255.) / 2.;
      a2 = ((dat_f[i+1] / FLT_MAX) * 255. + 255.) / 2.;
      a3 = ((dat_f[i+2] / FLT_MAX) * 255. + 255.) / 2.;
    } else if(sizeof(T) == 8) {
      a1 = ((dat_f[i+0] / DBL_MAX) * 255. + 255.) / 2.;
      a2 = ((dat_f[i+1] / DBL_MAX) * 255. + 255.) / 2.;
      a3 = ((dat_f[i+2] / DBL_MAX) * 255. + 255.) / 2.;
    } else {
      abort();
    }
    
    if(isnan(dat_f[i+0])) a1 = 255;
    if(isnan(dat_f[i+1])) a2 = 255;
    if(isnan(dat_f[i+2])) a3 = 255;
    
    if(isnan(dat_f[i+0])) { if(signbit(dat_f[i+0])) { a1 = 0; } else { a1 = 255; } }
    if(isnan(dat_f[i+1])) { if(signbit(dat_f[i+1])) { a2 = 0; } else { a2 = 255; } } 
    if(isnan(dat_f[i+2])) { if(signbit(dat_f[i+2])) { a3 = 0; } else { a3 = 255; } }
    
    if(isinf(dat_f[i+0])) { if(signbit(dat_f[i+0])) { a1 = 0; } else { a1 = 255; } }
    if(isinf(dat_f[i+1])) { if(signbit(dat_f[i+1])) { a2 = 0; } else { a2 = 255; } } 
    if(isinf(dat_f[i+2])) { if(signbit(dat_f[i+2])) { a3 = 0; } else { a3 = 255; } }
   
/* 
    if(a1 < 0 || a1 > 255) {
      printf("0 %d %f\n", a1, dat_f[i+0]);
    }
    if(a3 < 0 || a3 > 255) {
      printf("2 %d %f\n", a3, dat_f[i+2]);
    }
    if(a2 < 0 || a2 > 255) {
      printf("1 %d %f %d %d\n", a2, dat_f[i+1], isnan(dat_f[i+1]), dat_f[i+1] < 0);
    }
*/

    if(a1 < 0) a1 = 0;
    if(a2 < 0) a2 = 0;
    if(a3 < 0) a3 = 0;
    
    if(a1 > 255) a1 = 255;
    if(a2 > 255) a2 = 255;
    if(a3 > 255) a3 = 255;
    
    hist[a1 * 256 * 256 + a2 * 256 + a3]++;
  }
}

int *View3D::generate_histo(const unsigned char *dat_u8, long n, dtype_t dtype) {
  int *hist = new int[256 * 256 * 256];
  memset(hist, 0, sizeof(hist[0]) * 256 * 256 * 256);

  int st = overlap_->isChecked() ? 1 : 3;
  
  switch(dtype) {
  case none:
    break;
  case u8:
    {
      for(long i=0; i<n-2; i+=st) {
        int a1 = dat_u8[i+0];
        int a2 = dat_u8[i+1];
        int a3 = dat_u8[i+2];
        
        hist[a1 * 256 * 256 + a2 * 256 + a3]++;
      }
    }
    break;
  case u12:
    {
      const unsigned short *dat_u16 = (const unsigned short*)dat_u8;
      for(long i=0; i<n/2-2; i+=st) {
        int a1 = (dat_u16[i+0] & 0x0fff) / float(0x0fff) * 255.;
        int a2 = (dat_u16[i+1] & 0x0fff) / float(0x0fff) * 255.;
        int a3 = (dat_u16[i+2] & 0x0fff) / float(0x0fff) * 255.;

        hist[a1 * 256 * 256 + a2 * 256 + a3]++;
      }
    }
    break;
  case u16:
    {
      const unsigned short *dat_u16 = (const unsigned short*)dat_u8;
      for(long i=0; i<n/2-2; i+=st) {
        int a1 = dat_u16[i+0] / float(0xffff) * 255.;
        int a2 = dat_u16[i+1] / float(0xffff) * 255.;
        int a3 = dat_u16[i+2] / float(0xffff) * 255.;
        
        hist[a1 * 256 * 256 + a2 * 256 + a3]++;
      }
    }
    break;
  case u32:
    {
      const unsigned int *dat_u32 = (const unsigned int*)dat_u8;
      for(long i=0; i<n/4-2; i+=st) {
        int a1 = dat_u32[i+0] / float(0xffffffff) * 255.;
        int a2 = dat_u32[i+1] / float(0xffffffff) * 255.;
        int a3 = dat_u32[i+2] / float(0xffffffff) * 255.;
    
        hist[a1 * 256 * 256 + a2 * 256 + a3]++;
      }
    }
    break;
  case u64:
    {
      const unsigned long *dat_u64 = (const unsigned long*)dat_u8;
      for(long i=0; i<n/8-2; i+=st) {
        int a1 = dat_u64[i+0] / float(0xffffffffffffffff) * 255.;
        int a2 = dat_u64[i+1] / float(0xffffffffffffffff) * 255.;
        int a3 = dat_u64[i+2] / float(0xffffffffffffffff) * 255.;
    
        hist[a1 * 256 * 256 + a2 * 256 + a3]++;
      }
    }
    break;
  case f32:
    {
      const float *dat_f32 = (const float*)dat_u8;
      hist_float_helper_3d(hist, dat_f32, n, st);
    }
  case f64:
    {
      const double *dat_f64 = (const double*)dat_u8;
      hist_float_helper_3d(hist, dat_f64, n, st);
    }    
    break;
  }
 
#if 0 
  n_vertices = 0;
  float m=10000000, M=-1, a=0.;
  for(int i=0; i<256*256*256; i++) {
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

void View3D::setData(const unsigned char *dat, long n) {
  dat_ = dat;
  dat_n_ = n;

  regen_histo();
}

void View3D::initializeGL() {
  glClearColor(0, 0, 0, 0);
  glEnable(GL_DEPTH_TEST);
}

void View3D::resizeGL(int /*w*/, int /*h*/) {
  glMatrixMode(GL_PROJECTION);

  glLoadIdentity();
  gluPerspective(20, width() / (float) height(), 5, 15);
  glViewport(0, 0, width(), height());

  glMatrixMode(GL_MODELVIEW);
}

void View3D::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  glTranslatef(0, 0, -10);
  glRotatef(30, 1, 0, 0);
  glRotatef(alpha, 0, 1, 0);
  glRotatef(alpha2, 1, 0, 1);

  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_COLOR_ARRAY);
  if(1) {
    // Start at <-1, -1, -1>, and flip the sign on one dimension to produce a new unique point, continue until all paths terminate at <1,1,1>
    GLfloat lines_vertices[] =  {
      -1, -1, -1,     1, -1, -1,
      -1, -1, -1,    -1,  1, -1,
      -1, -1, -1,    -1, -1,  1,
      
       1, -1, -1,     1,  1, -1,
       1, -1, -1,     1, -1,  1,
      
      -1,  1, -1,     1,  1, -1,
      -1,  1, -1,    -1,  1,  1,
      
      -1, -1,  1,      1, -1, 1,
      -1, -1,  1,     -1,  1, 1,
      
      -1,  1,  1,      1,  1, 1,       
       1, -1,  1,      1,  1, 1,       
       1,  1, -1,      1,  1, 1,
      
      -1.05, -1.05, -1.05,    -1+.05, -1-.05, -1-.05,
      -1.05, -1.05, -1.05,    -1-.05, -1+.05, -1-.05,
      -1.05, -1.05, -1.05,    -1-.05, -1-.05, -1+.05
    };
    
    // slightly offset the edges from the data
    for(int i=0; i<24*3; i++) {
      if(lines_vertices[i] < 0) { lines_vertices[i] -= .01; }
      if(lines_vertices[i] > 0) { lines_vertices[i] += .01; }
    }
    
    GLfloat lines_colors[] = {
      .2, .2, .2,    .2, .2, .2,
      .2, .2, .2,    .2, .2, .2,
      .2, .2, .2,    .2, .2, .2,
      
      .2, .2, .2,    .2, .2, .2,
      .2, .2, .2,    .2, .2, .2,
      
      .2, .2, .2,    .2, .2, .2,
      .2, .2, .2,    .2, .2, .2,
      
      .2, .2, .2,    .2, .2, .2,
      .2, .2, .2,    .2, .2, .2,
      
      .2, .2, .2,    .2, .2, .2,
      .2, .2, .2,    .2, .2, .2,
      .2, .2, .2,    .2, .2, .2,
      
      .4, .4, .4,    .4, .4, .4,
      .4, .4, .4,    .4, .4, .4,
      .4, .4, .4,    .4, .4, .4
    };
    
    glVertexPointer(3, GL_FLOAT, 0, lines_vertices);
    glColorPointer(3, GL_FLOAT, 0, lines_colors);
    
    glDrawArrays(GL_LINES, 0, 24 + 6);
  }
  
  {
    if(vertices) {
      glVertexPointer(3, GL_FLOAT, 0, vertices);
      glColorPointer(3, GL_FLOAT, 0, colors);
    
      glDrawArrays(GL_POINTS, 0, n_vertices);
    }
  }
  
  glDisableClientState(GL_COLOR_ARRAY);
  glDisableClientState(GL_VERTEX_ARRAY);

  alpha = alpha + 0.1 * 20;
  alpha2 = alpha2 + 0.01 * 20;

  glFlush();
}

void View3D::regen_histo() {
  if(hist_) {
    delete[] hist_;
    hist_ = NULL;
  }

  dtype_t t;
  QString s = type_->currentText();
  if(s == "U8") t = u8;
  else if(s == "U12") t = u12;
  else if(s == "U16") t = u16;
  else if(s == "U32") t = u32;
  else if(s == "U64") t = u64;
  else if(s == "F32") t = f32;
  else if(s == "F64") t = f64;
  else t = none;

  hist_ = generate_histo(dat_, dat_n_, t);

  parameters_changed();
}

void View3D::parameters_changed() {
  int thresh = thresh_->value();
  float scale_factor = scale_->value();
  
  n_vertices = 0;
  for(int i=0; i<256*256*256; i++) {
    if(hist_[i] >= thresh) {
      n_vertices++;
    }
  }
  if(vertices) {
    delete[] vertices;
    delete[] colors;
    
    vertices = NULL;
    colors = NULL;
  }
  if(n_vertices > 0) {
    vertices = new GLfloat[n_vertices * 3];
    colors = new GLfloat[n_vertices * 3];
    for(int i=0, j=0; i<256*256*256; i++) {
      if(hist_[i] >= thresh) {
        float x = i / (256 * 256);
        float y = (i % (256*256)) / 256;
        float z = i % 256;
        x = x / 255.;
        y = y / 255.;
        z = z / 255.;
        /*
        if(x < -1 || x > 1.0 ||
           y < -1 || y > 1.0 ||
           z < -1 || z > 1.0) {
          printf("Warning 3dpc %f %f %f\n", x, y, z);
        }
        */
        vertices[j*3+0] = x * 2. - 1.;
        vertices[j*3+1] = y * 2. - 1.;
        vertices[j*3+2] = z * 2. - 1.;
    
        float cc = hist_[i] / scale_factor;
        cc += .2;
        if(cc > 1.) cc = 1.;
        colors[j*3+0] = cc;
        colors[j*3+1] = cc;
        colors[j*3+2] = cc;
        j++;
      }
    }
  }

  updateGL();
}
