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

#include <cfloat>
#include <QtGui>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>

#include <GL/glut.h>

#include "histogram.h"
#include "view_3d.h"

using std::isnan;
using std::signbit;
using std::isinf;


View3D::View3D(QWidget *p)
        : QGLWidget(p), hist_(NULL), dat_(NULL), dat_n_(0), spinning_(true) {
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
    if (hist_) delete[] hist_;
}

static float alpha = 0;
static float alpha2 = 0;

static GLfloat *vertices = NULL;
static GLfloat *colors = NULL;
int n_vertices = 0;

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
    if (1) {
        // Start at <-1, -1, -1>, and flip the sign on one dimension to produce a new unique point, continue until all paths terminate at <1,1,1>
        GLfloat lines_vertices[] = {
                -1, -1, -1, 1, -1, -1,
                -1, -1, -1, -1, 1, -1,
                -1, -1, -1, -1, -1, 1,

                1, -1, -1, 1, 1, -1,
                1, -1, -1, 1, -1, 1,

                -1, 1, -1, 1, 1, -1,
                -1, 1, -1, -1, 1, 1,

                -1, -1, 1, 1, -1, 1,
                -1, -1, 1, -1, 1, 1,

                -1, 1, 1, 1, 1, 1,
                1, -1, 1, 1, 1, 1,
                1, 1, -1, 1, 1, 1,

                -1.05, -1.05, -1.05, -1 + .05, -1 - .05, -1 - .05,
                -1.05, -1.05, -1.05, -1 - .05, -1 + .05, -1 - .05,
                -1.05, -1.05, -1.05, -1 - .05, -1 - .05, -1 + .05
        };

        // slightly offset the edges from the data
        for (int i = 0; i < 24 * 3; i++) {
            if (lines_vertices[i] < 0) { lines_vertices[i] -= .01; }
            if (lines_vertices[i] > 0) { lines_vertices[i] += .01; }
        }

        GLfloat lines_colors[] = {
                .2, .2, .2, .2, .2, .2,
                .2, .2, .2, .2, .2, .2,
                .2, .2, .2, .2, .2, .2,

                .2, .2, .2, .2, .2, .2,
                .2, .2, .2, .2, .2, .2,

                .2, .2, .2, .2, .2, .2,
                .2, .2, .2, .2, .2, .2,

                .2, .2, .2, .2, .2, .2,
                .2, .2, .2, .2, .2, .2,

                .2, .2, .2, .2, .2, .2,
                .2, .2, .2, .2, .2, .2,
                .2, .2, .2, .2, .2, .2,

                .4, .4, .4, .4, .4, .4,
                .4, .4, .4, .4, .4, .4,
                .4, .4, .4, .4, .4, .4
        };

        glVertexPointer(3, GL_FLOAT, 0, lines_vertices);
        glColorPointer(3, GL_FLOAT, 0, lines_colors);

        glDrawArrays(GL_LINES, 0, 24 + 6);
    }

    {
        if (vertices) {
            glVertexPointer(3, GL_FLOAT, 0, vertices);
            glColorPointer(3, GL_FLOAT, 0, colors);

            glDrawArrays(GL_POINTS, 0, n_vertices);
        }
    }

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);

    if (spinning_) {
        alpha = alpha + 0.1 * 20;
        alpha2 = alpha2 + 0.01 * 20;
    }

    glFlush();
}

void View3D::regen_histo() {
    if (hist_) {
        delete[] hist_;
        hist_ = NULL;
    }

    histo_dtype_t t = string_to_histo_dtype(type_->currentText().toStdString());

    hist_ = generate_histo_3d(dat_, dat_n_, t, overlap_->isChecked());

    parameters_changed();
}

void View3D::parameters_changed() {
    int thresh = thresh_->value();
    float scale_factor = scale_->value();

    n_vertices = 0;
    for (int i = 0; i < 256 * 256 * 256; i++) {
        if (hist_[i] >= thresh) {
            n_vertices++;
        }
    }
    if (vertices) {
        delete[] vertices;
        delete[] colors;

        vertices = NULL;
        colors = NULL;
    }
    if (n_vertices > 0) {
        vertices = new GLfloat[n_vertices * 3];
        colors = new GLfloat[n_vertices * 3];
        for (int i = 0, j = 0; i < 256 * 256 * 256; i++) {
            if (hist_[i] >= thresh) {
                float x = i / (256 * 256);
                float y = (i % (256 * 256)) / 256;
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
                vertices[j * 3 + 0] = x * 2. - 1.;
                vertices[j * 3 + 1] = y * 2. - 1.;
                vertices[j * 3 + 2] = z * 2. - 1.;

                float cc = hist_[i] / scale_factor;
                cc += .2;
                if (cc > 1.) cc = 1.;
                colors[j * 3 + 0] = cc;
                colors[j * 3 + 1] = cc;
                colors[j * 3 + 2] = cc;
                j++;
            }
        }
    }

    updateGL();
}

void View3D::mousePressEvent(QMouseEvent *e) {
    e->accept();
}

void View3D::mouseMoveEvent(QMouseEvent *e) {
    e->accept();
}

void View3D::mouseReleaseEvent(QMouseEvent *e) {
    e->accept();
    spinning_ = !spinning_;
}
