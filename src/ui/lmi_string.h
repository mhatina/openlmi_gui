/* ***** BEGIN LICENSE BLOCK *****
 *
 *   Copyright (C) 2014, Peter Hatina <phatina@redhat.com>
 *                       Martin Hatina <mhatina@redhat.com>
 *
 *   This library is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License as
 *   published by the Free Software Foundation, either version 2.1 of the
 *   License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *   MA 02110-1301 USA
 *
 * ***** END LICENSE BLOCK ***** */

#  ifndef STRING_H
#  define STRING_H

#  include <string>
#  include <Pegasus/Common/String.h>
#  include <QString>

class String: public std::string
{
public:
    String();
    String(const char *str);
    String(const char *str, size_t pos, size_t len = npos);
    String(const std::string &str);
    String(const Pegasus::String &str);
    String(const QString &str);

    // Unification methods
    const char *asConstChar() const;
    std::string asStdString() const;
    Pegasus::String asPegasusString() const;
    QString asQString() const;

    operator const char *() const;
    operator Pegasus::String() const;
    operator QString() const;

    String &operator=(const char *rhs);
    String &operator=(const String &rhs);
    String &operator=(const std::string &rhs);
    String &operator=(const Pegasus::String &rhs);
    String &operator=(const QString &rhs);

    String &operator+=(const char *);
    String &operator+=(const Pegasus::String &rhs);
    String &operator+=(const QString &rhs);
    String &operator+=(const String &rhs);

    String &operator+(const char *rhs);
    String &operator+(const String &rhs);
    String &operator+(const std::string &rhs);
    String &operator+(const Pegasus::String &rhs);
    String &operator+(const QString &rhs);

    friend String &operator+(const char *rhs, String &str);
    friend String &operator+(const std::string &rhs, String &str);
    friend String &operator+(const Pegasus::String &rhs, String &str);
    friend String &operator+(const QString &rhs, String &str);

    using std::string::npos;
};

String &operator+(const char *rhs, String &str);
String &operator+(const std::string &rhs, String &str);
String &operator+(const Pegasus::String &rhs, String &str);
String &operator+(const QString &rhs, String &str);

#endif // STRING_H
