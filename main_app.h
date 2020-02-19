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

#ifndef _MAIN_APP_H_
#define _MAIN_APP_H_

#include <QDialog>

class ImageView;

class ImageView2;

class ImageView3;

class DotPlot;

class View3D;

class GraphView;

class QLabel;

class MainApp : public QDialog {
Q_OBJECT
public:
    explicit MainApp(QWidget *p = nullptr);

    ~MainApp() override;

    bool load_file(const QString &filename);

    bool load_files(const QStringList &filenames);

public slots:

    void reject() override;

protected slots:

    void quit();

    void rangeSelected(float, float);

    void switchView(int);

    void loadFile();

    bool prevFile();

    bool nextFile();

protected:
    ImageView *iv1_;
    ImageView *iv2_;
    GraphView *iv2e_;
    ImageView2 *iv2d_;
    ImageView3 *iv2d2_;
    DotPlot *dot_plot_;
    View3D *v3d_;
    std::vector<QWidget *> views_;

    QLabel *filename_;
    QStringList files_;
    int cur_file_;

    unsigned char *bin_;
    size_t bin_len_;

    bool done_flag_;

    size_t start_;
    size_t end_;

//    void updatePositions(bool resized = false);

    void resizeEvent(QResizeEvent *e) override;

    void update_views(bool update_iv1 = true);
};

#endif
