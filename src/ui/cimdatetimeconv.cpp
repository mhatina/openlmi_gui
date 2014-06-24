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

#include "cimdatetimeconv.h"

#include <sstream>


std::string CIMDateTimeConv::as_time_value(
    const Pegasus::CIMDateTime &dt)
{
    std::string timestr(dt.toString().getCString());

    std::stringstream ss;
    ss << timestr.substr(8, 2) << ":" << timestr.substr(10, 2) << ":" <<
          timestr.substr(12, 2) << " " << timestr.substr(6, 2) << "." <<
          timestr.substr(4, 2) << "." << timestr.substr(0, 4);

    return ss.str();
}
