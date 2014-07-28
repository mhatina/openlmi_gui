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
#include "logger.h"

#include <sstream>
#include <ctime>
#include <Pegasus/Common/Array.h>

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

template <>
std::string get_pegasus_value_core(const Pegasus::Uint8 &value)
{
    std::stringstream ss;
    ss << static_cast<Pegasus::Uint16>(value);
    return ss.str();
}

template <>
std::string get_pegasus_value_core(const Pegasus::Sint8 &value)
{
    std::stringstream ss;
    ss << static_cast<Pegasus::Sint16>(value);
    return ss.str();
}

template <>
std::string get_pegasus_value_core(const Pegasus::Boolean &value)
{
    return value ? "True" : "False";
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
        ss << "{";
        const Pegasus::Uint32 cnt = value.getArraySize();
        for (Pegasus::Uint32 i = 0; i < cnt; ++i) {
            const T &raw_value = raw_array[i];
            ss << get_pegasus_value_core<T>(raw_value);
            if (i < cnt - 1)
                ss << ", ";
        }
        ss << "}";
        return ss.str();
    }
}

template <typename T>
T get_value_from_std_string_core(const std::string &value)
{
    T v;
    std::stringstream ss(value);
    ss >> v;
    return v;
}

template <>
Pegasus::Char16 get_value_from_std_string_core(const std::string &value)
{
    return get_value_from_std_string_core<Pegasus::Uint16>(value);
}

template <>
Pegasus::String get_value_from_std_string_core(const std::string &value)
{
    return value.c_str();
}

template <>
Pegasus::CIMDateTime get_value_from_std_string_core(const std::string &value)
{
    return CIMDateTimeConv::as_cim_date_time(value);
}

template <typename T>
Pegasus::CIMValue get_value_from_std_string(std::string &value, bool isArray)
{
    if (!isArray)
        return Pegasus::CIMValue(get_value_from_std_string_core<T>(value));

    if (value[0] == '{') {
        value = value.substr(1, value.size() - 2);
    }

    int pos = 0;
    int pos_start = 0;
    Pegasus::Array<T> array;
    while (1) {
        pos = value.find(",");
        std::string tmp = value.substr(pos_start, pos);
        array.append(get_value_from_std_string_core<T>(tmp));
        if (pos == std::string::npos)
            break;

        pos_start = pos + 1;
        while (pos_start < value.size() && isspace(value[pos_start]))
            pos_start++;
        value = value.substr(pos_start);
        pos_start = 0;
    }
    return Pegasus::CIMValue(array);
}

} // unnamed namespace

std::string CIMValue::to_std_string(const Pegasus::CIMValue &value)
{

    Logger::getInstance()->debug("CIMValue::to_std_string(const Pegasus::CIMValue &value)");
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

#include <sstream>

Pegasus::CIMValue CIMValue::to_cim_value(Pegasus::CIMType type, std::string value, bool isArray)
{
    Logger::getInstance()->debug("CIMValue::to_cim_value(Pegasus::CIMType type, std::string value, bool isArray)");
    switch (type) {
    case Pegasus::CIMTYPE_BOOLEAN:
        return get_value_from_std_string<Pegasus::Boolean>(value, isArray);
    case Pegasus::CIMTYPE_UINT8:
        return get_value_from_std_string<Pegasus::Uint8>(value, isArray);
    case Pegasus::CIMTYPE_SINT8:
        return get_value_from_std_string<Pegasus::Sint8>(value, isArray);
    case Pegasus::CIMTYPE_UINT16:
        return get_value_from_std_string<Pegasus::Uint16>(value, isArray);
    case Pegasus::CIMTYPE_SINT16:
        return get_value_from_std_string<Pegasus::Sint16>(value, isArray);
    case Pegasus::CIMTYPE_UINT32:
        return get_value_from_std_string<Pegasus::Uint32>(value, isArray);
    case Pegasus::CIMTYPE_SINT32:
        return get_value_from_std_string<Pegasus::Sint32>(value, isArray);
    case Pegasus::CIMTYPE_UINT64:
        return get_value_from_std_string<Pegasus::Uint64>(value, isArray);
    case Pegasus::CIMTYPE_SINT64:
        return get_value_from_std_string<Pegasus::Sint64>(value, isArray);
    case Pegasus::CIMTYPE_CHAR16:
        return get_value_from_std_string<Pegasus::Char16>(value, isArray);
    case Pegasus::CIMTYPE_STRING:
        return get_value_from_std_string<Pegasus::String>(value, isArray);
    case Pegasus::CIMTYPE_DATETIME:
        return get_value_from_std_string<Pegasus::CIMDateTime>(value, isArray);
//    case Pegasus::CIMTYPE_REFERENCE:
//        return get_pegasus_value<Pegasus::CIMObjectPath>(value);
//    case Pegasus::CIMTYPE_OBJECT:
//        return get_pegasus_value<Pegasus::CIMObject>(value);
    default:
        return Pegasus::CIMValue();
    }
}
