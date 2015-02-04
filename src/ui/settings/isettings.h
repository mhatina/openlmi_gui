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

#ifndef ISETTINGS_H
#define ISETTINGS_H

#include "lmi_string.h"

#include <map>
#include <QFile>
#include <QWidget>
#include <QXmlStreamWriter>

class ISettings : public QWidget
{
    Q_OBJECT

public:
    explicit ISettings(QWidget *parent = 0);
    virtual String title() = 0;
    virtual void init() = 0;
    virtual void load(QFile &file) = 0;
    virtual void save(QXmlStreamWriter &writer) = 0;

signals:

public slots:

};

#endif // ISETTINGS_H
