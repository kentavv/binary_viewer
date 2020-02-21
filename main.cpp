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

#include <qapplication.h>

#include "main_app.h"
#include "version.h"


//----------------------------------------------------------------------

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainApp a;

    if (argc > 2) {
        fprintf(stderr, "usage: %s <filename>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (argc == 2) {
        if (!a.load_file(argv[1])) {
            exit(EXIT_FAILURE);
        }
    }

    a.showFullScreen();
    a.setWindowTitle(base_caption);

    return a.exec();
}
