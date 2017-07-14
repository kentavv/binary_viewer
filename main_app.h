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

#ifndef _MAIN_APP_H_
#define _MAIN_APP_H_

#include <QDialog>

class ImageView;
class ImageView2;
class ImageView3;
class DotPlot;
class View3D;
class GraphView;

class MainApp : public QDialog {
  Q_OBJECT
  public:
  MainApp(QWidget *p=NULL);
  ~MainApp();
  
  bool load_file(const char *filename);

public slots:
  void reject();
               
protected slots:
  void quit();
  void rangeSelected(float, float);
  void switchView();

protected:
  ImageView *iv1_;
  ImageView *iv2_;
  GraphView *iv2e_;
  ImageView2 *iv2d_;
  ImageView3 *iv2d2_;
  DotPlot *dot_plot_;
  View3D *v3d_;

  unsigned char *bin_;
  long bin_len_;
  
  bool done_flag_;

  QString euid;

  long start_;
  long end_;
  
  void updatePositions(bool resized=false);
  void resizeEvent(QResizeEvent *e);
  void update_views(bool update_iv1 = true);
};

#endif
