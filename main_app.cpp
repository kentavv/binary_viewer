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

#include <unistd.h>
#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <QtGui>

#include "main_app.h"
#include "image_view.h"
#include "image_view2.h"
#include "image_view3.h"
#include "dot_plot.h"
#include "view_3d.h"
#include "graph_view.h"

using std::min;

extern const QString base_caption;
extern QString caption;

static int scroller_w = 16*8;

MainApp::MainApp(QWidget *p)
  : QDialog(p), bin_(NULL), bin_len_(0), start_(0), end_(0)
{
  if(1) {
    QPalette pal = palette();
    pal.setBrush(QPalette::Background, QBrush(Qt::black));
    setPalette(pal);
  }
  
  done_flag_ = false;

  iv1_ = new ImageView;
  iv2_ = new ImageView;
  iv2e_ = new GraphView;
  v3d_ = new View3D;
  iv2d_ = new ImageView2;
  iv2d2_ = new ImageView3;
  dot_plot_ = new DotPlot;
  
  connect(iv1_, SIGNAL(rangeSelected(float, float)), SLOT(rangeSelected(float, float)));

  iv1_->setFixedWidth(scroller_w);
  iv2_->setFixedWidth(scroller_w);
  iv2e_->setFixedWidth(scroller_w);

  iv2_->enableSelection(false);
  iv2e_->enableSelection(false);

  QHBoxLayout *layout = new QHBoxLayout;
  layout->addWidget(iv1_);
  layout->addWidget(iv2_);
  layout->addWidget(iv2e_);
  QPushButton *pb = new QPushButton("*");
  pb->setFixedSize(pb->sizeHint());
  connect(pb, SIGNAL(clicked()), SLOT(switchView()));
  layout->addWidget(pb);
  layout->addWidget(v3d_);
  layout->addWidget(iv2d_);
  layout->addWidget(iv2d2_);
  layout->addWidget(dot_plot_);

  v3d_->show();
  iv2d_->hide();
  iv2d2_->hide();
  dot_plot_->hide();
  
  setLayout(layout);
}

MainApp::~MainApp() {
  quit();
}

void MainApp::resizeEvent(QResizeEvent *e){
  QDialog::resizeEvent(e);
  update_views();
}

void MainApp::quit() {
  if(!done_flag_) {
    done_flag_ = true;

    exit(EXIT_SUCCESS);
  }
}

void MainApp::reject() {
  quit();
}

bool MainApp::load_file(const char *filename) {
  FILE *f = fopen(filename, "rb");
  if(!f) {
    fprintf(stderr, "Unable to open %s\n", filename);
    return false;
  }
  fseek(f, 0, SEEK_END);
  long len = ftell(f);
  fseek(f, 0, SEEK_SET);
  
  if(bin_ != NULL) {
    delete[] bin_;
    bin_ = NULL;
    bin_len_ = 0;
    start_ = 0;
    end_ = 0;
  }

  bin_ = new unsigned char[len];
  bin_len_ = fread(bin_, 1, len, f);
  fclose(f);

  if(len != bin_len_) {
    printf("premature read %ld of %ld\n", bin_len_, len);
  }
  
  start_ = 0;
  end_ = bin_len_;

  update_views();

  return true;
}

void MainApp::update_views(bool update_iv1) {
  if(update_iv1) iv1_->clear();

  if(bin_ == NULL) return;

  // img1 shows the entire file, img2 shows the current segment
  if(update_iv1) iv1_->set_data(bin_ + 0, bin_len_);
  iv2_->set_data(bin_ + start_, end_-start_);
  {
    //int bs = 1024;
    int bs = 256;
    //int bs = 1;
    int n = (end_-start_) / bs + 1;
    float *dd = new float[n];
    memset(dd, 0, n*sizeof(float));
    {
      for(long is=start_; is<end_; is+=bs) {
        long ie = min(end_, is+bs);
        int dict[256] = { 0 };
        for(long i=is; i<ie; i++) {
          dict[bin_[i]]++;
        }
        float entropy = 0.;
        for(int i=0; i<256; i++) {
          float p = dict[i] / float(ie - is);
          if(p > 0.) {
            entropy += -p * logf(p);
          }
        }
        entropy /= logf(2.0);
        entropy /= 8.0;
        int di = (is-start_)/bs;
        if(di >= n) {
          //printf("%d %d %d %d\n", is, bs, is/bs, n);
          continue;
        }
        dd[di] = entropy;
      }
    }
    iv2e_->set_data(dd, n);
    delete[] dd;
  }

  if(v3d_->isVisible()) v3d_->setData(bin_+start_, end_-start_);
  if(iv2d_->isVisible()) iv2d_->setData(bin_+start_, end_-start_);
  if(iv2d2_->isVisible()) iv2d2_->setData(bin_+start_, end_-start_);
  if(dot_plot_->isVisible()) dot_plot_->setData(bin_+start_, end_-start_);
}

void MainApp::rangeSelected(float s, float e) {
  start_ = s * bin_len_;
  end_ = e * bin_len_;
  update_views(false);
}

void MainApp::switchView() {
  if(v3d_->isVisible()) {
    v3d_->hide();
    iv2d_->show();
    iv2d2_->hide();
    dot_plot_->hide();
  } else if(iv2d_->isVisible()) {
    v3d_->hide();
    iv2d_->hide();
    iv2d2_->show();
    dot_plot_->hide();
  } else if(iv2d2_->isVisible()) {
    v3d_->hide();
    iv2d_->hide();
    iv2d2_->hide();
    dot_plot_->show();
  } else if(dot_plot_->isVisible()) {
    v3d_->show();
    iv2d_->hide();
    iv2d2_->hide();
    dot_plot_->hide();
  }
  update_views(false);
}
