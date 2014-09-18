/* ***** BEGIN LICENSE BLOCK *****
 *   Copyright (C) 2013-2014, Peter Hatina <phatina@redhat.com>,
 *                            Martin Hatina <mhatina@redhat.com>
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

#ifndef LMIWBEM_VALUE_H
#define LMIWBEM_VALUE_H

#include <string>
#include <Pegasus/Common/CIMValue.h>

/**
 * @brief The CIMValue class
 *
 * Convert CIMValue to better std::string
 */
class CIMValue
{

public:
    static std::string decode_values(Pegasus::CIMProperty property);
    /**
     * @brief Convert CIMValue
     * @param value -- value to convert
     * @return std::string representation of value
     */
    static std::string to_std_string(const Pegasus::CIMValue &value);
    static std::string get_property_value(Pegasus::CIMInstance instance,
                                          std::string propertyName, Pegasus::CIMProperty *property = NULL);
    static Pegasus::CIMValue to_cim_value(Pegasus::CIMType type, std::string value,
                                          bool isArray = false);

};


#endif // LMIWBEM_VALUE_H
