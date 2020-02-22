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
#include "image_view.h"
#include "image_view2.h"
#include "image_view3.h"
#include "dot_plot.h"
#include "view_3d.h"
#include "graph_view.h"
#include "histogram.h"

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
        iv1_ = new ImageView;
        iv2_ = new ImageView;
        iv2e_ = new GraphView;

        connect(iv1_, SIGNAL(rangeSelected(float, float)), SLOT(rangeSelected(float, float)));

        iv1_->setFixedWidth(scroller_w);
        iv2_->setFixedWidth(scroller_w);
        iv2e_->setFixedWidth(scroller_w);

        iv2_->enableSelection(false);
        iv2e_->enableSelection(false);

        auto layout = new QHBoxLayout;
        layout->addWidget(iv1_);
        layout->addWidget(iv2_);
        layout->addWidget(iv2e_);
        top_layout->addLayout(layout, 1, 0);
    }

    {
        auto layout = new QHBoxLayout;

        {
            auto pb = new QComboBox();
            pb->addItem("3D correlation");
            pb->addItem("2D correlation");
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
        v3d_ = new View3D;
        iv2d_ = new ImageView2;
        iv2d2_ = new ImageView3;
        dot_plot_ = new DotPlot;

        views_.push_back(v3d_);
        views_.push_back(iv2d_);
        views_.push_back(iv2d2_);
        views_.push_back(dot_plot_);

        auto layout = new QHBoxLayout;
        for (const auto &j : views_) {
            layout->addWidget(j);
        }

        top_layout->addLayout(layout, 1, 1);
    }

    v3d_->show();
    iv2d_->hide();
    iv2d2_->hide();
    dot_plot_->hide();

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
    if (update_iv1) iv1_->clear();

    if (bin_ == nullptr) return;

    // iv1 shows the entire file, iv2 shows the current segment
    if (update_iv1) iv1_->set_data(bin_ + 0, bin_len_);
    iv2_->set_data(bin_ + start_, end_ - start_);

    {
        long n;
        auto dd = generate_entropy(bin_ + start_, end_ - start_, n);
        if (dd != nullptr) {
            iv2e_->set_data(0, dd, n);
            delete[] dd;
        }
    }

    {
        auto dd = generate_histo(bin_ + start_, end_ - start_);
        if (dd != nullptr) {
            iv2e_->set_data(1, dd, 256, false);
            delete[] dd;
        }
    }

    if (v3d_->isVisible()) v3d_->setData(bin_ + start_, end_ - start_);
    if (iv2d_->isVisible()) iv2d_->setData(bin_ + start_, end_ - start_);
    if (iv2d2_->isVisible()) iv2d2_->setData(bin_ + start_, end_ - start_);
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
