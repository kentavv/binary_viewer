/*
 * Copyright (c) 2015, 2017, 2020 Kent A. Vander Velden, kent.vandervelden@gmail.com
 *
 * This file is part of BinVis.
 *
 *     BinVis is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 *
 *     BinVis is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 *
 *     You should have received a copy of the GNU General Public License
 *     along with BinVis.  If not, see <https://www.gnu.org/licenses/>.
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
