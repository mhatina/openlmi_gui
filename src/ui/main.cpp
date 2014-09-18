/* ***** BEGIN LICENSE BLOCK *****
 *   Copyright (C) 2013-2014, Martin Hatina <mhatina@redhat.com>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 * ***** END LICENSE BLOCK ***** */

#define _POSIX_C_SOURCE

#include "kernel.h"
#include "logger.h"

#include <QApplication>
#include <unistd.h>

void usage(const char *arg)
{
    const char *name = strrchr(arg, '/');
    std::cerr << "Usage: " << name << " [-g]\n\n"
              << "Options:\n"
              << "-g\t-- show debug messages\n"
              << "-h\t-- show this help";
}

int main(int argc, char *argv[])
{
    Logger::getInstance()->debug("main(int argc, char *argv[])");
    int opt;
    const char *shortopts = "gh";
    while ((opt = getopt(argc, argv, shortopts)) != -1)
        switch (opt) {
        case 'g':
            Logger::getInstance()->setShowDebug(true);
            break;
        case 'h':
        case '?':
            usage(argv[0]);
            return 1;
        default:
            break;
        }


    QApplication a(argc, argv);

    Engine::Kernel kernel;
    kernel.showMainWindow();

    return a.exec();
}
