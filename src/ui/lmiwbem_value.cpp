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

#include "lmiwbem_value.h"
#include "cimdatetimeconv.h"

#include <sstream>
#include <ctime>

namespace {

/**
 * @brief get_pegasus_value_core
 * @param value
 * @return
 */
template <typename T>
std::string get_pegasus_value_core(const T &value)
{
    std::stringstream ss;
    ss << value;
    return ss.str(); 
}

/**
 * @brief get_pegasus_value_core
 * @param value
 * @return
 */
template <>
std::string get_pegasus_value_core(const Pegasus::CIMDateTime &value)
{
    return CIMDateTimeConv::as_time_value(value);
}

/**
 * @brief get_pegasus_value
 * @param value
 * @return
 */
template <typename T>
std::string get_pegasus_value(const Pegasus::CIMValue &value)
{
    if (value.isNull())
        return std::string(); 

    if (!value.isArray()) {
        T raw_value;        
        value.get(raw_value);
        return get_pegasus_value_core<T>(raw_value);
    } else {
        Pegasus::Array<T> raw_array;
        std::stringstream ss;
        value.get(raw_array);
        const Pegasus::Uint32 cnt = value.getArraySize();
        for (Pegasus::Uint32 i = 0; i < cnt; ++i) {
            const T &raw_value = raw_array[i];
            ss << get_pegasus_value_core<T>(raw_value);
            if (i < cnt - 1)
                ss << ", ";
        }
        return ss.str();
    }
}

} // unnamed namespace

std::string CIMValue::to_std_string(const Pegasus::CIMValue &value)
{
    switch (value.getType()) {
    case Pegasus::CIMTYPE_BOOLEAN:
        return get_pegasus_value<Pegasus::Boolean>(value);
    case Pegasus::CIMTYPE_UINT8:
        return get_pegasus_value<Pegasus::Uint8>(value);
    case Pegasus::CIMTYPE_SINT8:
        return get_pegasus_value<Pegasus::Sint8>(value);
    case Pegasus::CIMTYPE_UINT16:
        return get_pegasus_value<Pegasus::Uint16>(value);
    case Pegasus::CIMTYPE_SINT16:
        return get_pegasus_value<Pegasus::Sint16>(value);
    case Pegasus::CIMTYPE_UINT32:
        return get_pegasus_value<Pegasus::Uint32>(value);
    case Pegasus::CIMTYPE_SINT32:
        return get_pegasus_value<Pegasus::Sint32>(value);
    case Pegasus::CIMTYPE_UINT64:
        return get_pegasus_value<Pegasus::Uint64>(value);
    case Pegasus::CIMTYPE_SINT64:
        return get_pegasus_value<Pegasus::Sint64>(value);
    case Pegasus::CIMTYPE_CHAR16:
        return get_pegasus_value<Pegasus::Char16>(value);
    case Pegasus::CIMTYPE_STRING:
        return get_pegasus_value<Pegasus::String>(value);
    case Pegasus::CIMTYPE_DATETIME:
        return get_pegasus_value<Pegasus::CIMDateTime>(value);
//    case Pegasus::CIMTYPE_REFERENCE:
//        return get_pegasus_value<Pegasus::CIMObjectPath>(value);
//    case Pegasus::CIMTYPE_OBJECT:
//        return get_pegasus_value<Pegasus::CIMObject>(value);
    default:
        return std::string("Not implemented");
    }
}
