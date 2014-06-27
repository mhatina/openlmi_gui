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

#ifndef CIMDATETIMECONV_H
#define CIMDATETIMECONV_H

#include <Pegasus/Common/CIMDateTime.h>

/**
 * @brief The CIMDateTimeConv class
 *
 * Convert CIMDateTime to better readable format
 */
class CIMDateTimeConv
{
public:
    static Pegasus::CIMDateTime as_cim_date_time(const std::string &dt);
    /**
     * @brief Convert CIMDateTime
     * @param dt -- date
     * @return time in HH:MM:SS DD.MM.YYYY format
     */
    static std::string as_time_value(const Pegasus::CIMDateTime &dt);
};

#endif // CIMDATETIMECONV_H
