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

#include <cstdio>
#include <cstdlib>

#include <QtGui>
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>

#include "main_app.h"
#include "binary_viewer.h"
#include "overall_view.h"
#include "histogram_2d_view.h"
#include "image_view.h"
#include "dot_plot.h"
#include "histogram_3d_view.h"
#include "plot_view.h"
#include "histogram_calc.h"

static int scroller_w = 16 * 8;


MainApp::MainApp(QWidget *p)
        : QDialog(p), cur_file_(-1), bin_(nullptr), bin_len_(0), start_(0), end_(0) {
    done_flag_ = false;

    auto top_layout = new QGridLayout;

    {
        auto layout = new QHBoxLayout;
        {
            auto pb = new QPushButton("Load file");
            pb->setFixedSize(pb->sizeHint());
            connect(pb, SIGNAL(clicked()), SLOT(loadFile()));
            layout->addWidget(pb);
        }
        {
            auto pb = new QPushButton("Prev");
            pb->setFixedSize(pb->sizeHint());
            connect(pb, SIGNAL(clicked()), SLOT(prevFile()));
            layout->addWidget(pb);
        }
        {
            auto pb = new QPushButton("Next");
            pb->setFixedSize(pb->sizeHint());
            connect(pb, SIGNAL(clicked()), SLOT(nextFile()));
            layout->addWidget(pb);
        }
        top_layout->addLayout(layout, 0, 0);
    }

    {
        overall_primary_ = new OverallView;
        overall_zoomed_ = new OverallView;
        plot_view_ = new PlotView;

        connect(overall_primary_, SIGNAL(rangeSelected(float, float)), SLOT(rangeSelected(float, float)));

        overall_primary_->setFixedWidth(scroller_w);
        overall_zoomed_->setFixedWidth(scroller_w);
        plot_view_->setFixedWidth(scroller_w);

        overall_zoomed_->enableSelection(false);
        plot_view_->enableSelection(false);

        auto layout = new QHBoxLayout;
        layout->addWidget(overall_primary_);
        layout->addWidget(overall_zoomed_);
        layout->addWidget(plot_view_);
        top_layout->addLayout(layout, 1, 0);
    }

    {
        auto layout = new QHBoxLayout;

        {
            auto pb = new QComboBox();
            pb->addItem("3D histogram");
            pb->addItem("2D histogram");
            pb->addItem("Binary view");
            pb->addItem("Image view");
            pb->addItem("Dot plot");
            pb->setFixedSize(pb->sizeHint());
            connect(pb, SIGNAL(currentIndexChanged(int)), SLOT(switchView(int)));
            layout->addWidget(pb);
        }
        {
            filename_ = new QLabel();
            layout->addWidget(filename_);
        }

        top_layout->addLayout(layout, 0, 1);
    }

    {
        histogram_3d_ = new Histogram3dView;
        histogram_2d_ = new Histogram2dView;
        binary_viewer_ = new BinaryViewer;
        image_view_ = new ImageView;
        dot_plot_ = new DotPlot;

        views_.push_back(histogram_3d_);
        views_.push_back(histogram_2d_);
        views_.push_back(binary_viewer_);
        views_.push_back(image_view_);
        views_.push_back(dot_plot_);

        auto layout = new QHBoxLayout;
        for (const auto &j : views_) {
            layout->addWidget(j);
        }

        top_layout->addLayout(layout, 1, 1);
    }

    switchView(0);

    setLayout(top_layout);
}

MainApp::~MainApp() {
    quit();
}

void MainApp::resizeEvent(QResizeEvent *e) {
    QDialog::resizeEvent(e);
    update_views();
}

void MainApp::quit() {
    if (!done_flag_) {
        done_flag_ = true;

        exit(EXIT_SUCCESS);
    }
}

void MainApp::reject() {
    quit();
}

bool MainApp::load_file(const QString &filename) {
    QString title;
    if (files_.size() > 1) {
        title = QString("%1/%2: %3").arg(cur_file_ + 1).arg(files_.size()).arg(filename);
    } else {
        title = filename;
    }
    filename_->setText(title);

    FILE *f = fopen(filename.toStdString().c_str(), "rb");
    if (!f) {
        fprintf(stderr, "Unable to open %s\n", filename.toStdString().c_str());
        return false;
    }
    fseek(f, 0, SEEK_END);
    long len = ftell(f);
    fseek(f, 0, SEEK_SET);

    if (bin_ != nullptr) {
        delete[] bin_;
        bin_ = nullptr;
        bin_len_ = 0;
        start_ = 0;
        end_ = 0;
    }

    bin_ = new unsigned char[len];
    bin_len_ = fread(bin_, 1, len, f);
    fclose(f);

    if (len != bin_len_) {
        printf("premature read %ld of %ld\n", bin_len_, len);
    }

    start_ = 0;
    end_ = bin_len_;

    update_views();

    return true;
}

bool MainApp::load_files(const QStringList &filenames) {
    files_ = filenames;
    cur_file_ = -1;
    return nextFile();
}

bool MainApp::prevFile() {
    bool rv = false;
    while (cur_file_ > 0 && !rv) {
        cur_file_--;
        rv = load_file(files_[cur_file_]);
    }
    return rv;
}

bool MainApp::nextFile() {
    bool rv = false;
    while (cur_file_ + 1 < files_.size() && !rv) {
        cur_file_++;
        rv = load_file(files_[cur_file_]);
    }
    return rv;
}

void MainApp::loadFile() {
    QStringList files = QFileDialog::getOpenFileNames(
            this,
            "Select one or more files to open");
    if (!files.empty()) {
        load_files(files);
    }
}

void MainApp::update_views(bool update_iv1) {
    if (update_iv1) overall_primary_->clear();

    if (bin_ == nullptr) return;

    // iv1 shows the entire file, iv2 shows the current segment
    if (update_iv1) overall_primary_->set_data(bin_ + 0, bin_len_);
    overall_zoomed_->set_data(bin_ + start_, end_ - start_);

    {
        long n;
        auto dd = generate_entropy(bin_ + start_, end_ - start_, n);
        if (dd) {
            plot_view_->set_data(0, dd, n);
            delete[] dd;
        }
    }

    {
        auto dd = generate_histo(bin_ + start_, end_ - start_);
        if (dd) {
            plot_view_->set_data(1, dd, 256, false);
            delete[] dd;
        }
    }

    if (histogram_3d_->isVisible()) histogram_3d_->setData(bin_ + start_, end_ - start_);
    if (histogram_2d_->isVisible()) histogram_2d_->setData(bin_ + start_, end_ - start_);
    if (binary_viewer_->isVisible()) {
//        binary_viewer_->setData(bin_ + start_, end_ - start_);
        binary_viewer_->setData(bin_, end_);
        binary_viewer_->setStart(start_ / 16);
    }
    if (image_view_->isVisible()) image_view_->setData(bin_ + start_, end_ - start_);
    if (dot_plot_->isVisible()) dot_plot_->setData(bin_ + start_, end_ - start_);
}

void MainApp::rangeSelected(float s, float e) {
    start_ = s * bin_len_;
    end_ = e * bin_len_;
    update_views(false);
}

void MainApp::switchView(int ind) {
    for (const auto &j : views_) {
        j->hide();
    }
    views_[ind]->show();
    update_views(false);
}
