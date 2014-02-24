/*
 * Copyright 2013-2014 Paul Colby
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file
 * @brief Defines the ConsoleUtils class.
 */

#include "ConsoleUtils.h"

#include <qpid/console/Value.h>

#include <boost/lexical_cast.hpp>

/**
 * @brief Get a standardised name for a QMF object.
 *
 * This function checks a QMF object for an appropriate "name" attribute, and if
 * found, returns it verbatim.  If the object has no "name" attribute, and
 * allowNodeName is \c true, and the object has a "nodeName" attribute, then
 * that attribute's value will be returned verbatim.  Otherwise, and empty
 * string is returned.
 *
 * @param object        Object to fetch the name of.
 * @param allowNodeName If \c true, and \a object has no name attribute, the
 *                      nodeName attribute will be considered also.
 *
 * @return A name for \a object, or an empty string if no name could be found.
 */
std::string ConsoleUtils::getName(const qpid::console::Object &object,
                                  const bool allowNodeName)
{
    const qpid::console::Object::AttributeMap &attributes = object.getAttributes();
    qpid::console::Object::AttributeMap::const_iterator name = attributes.find("name");
    if ((name == attributes.end()) && (allowNodeName)) {
        name = attributes.find("nodeName");
    }
    return (name == attributes.end()) ? std::string() : name->second->asString();
}

/**
 * @brief Get the schema type of a QMF object.
 *
 * @param object QMF object to fetch the type of.
 *
 * @return One of the ConsoleUtils::ObjectSchemaType enumeration values.
 */
ConsoleUtils::ObjectSchemaType ConsoleUtils::getType(const qpid::console::Object &object)
{
    return getType(object.getClassKey());
}

/**
 * @brief Get the schema type of a QMF class key.
 *
 * @param classKey QMF class key to fetch the type of.
 *
 * @return One of the ConsoleUtils::ObjectSchemaType enumeration values.
 */
ConsoleUtils::ObjectSchemaType ConsoleUtils::getType(const qpid::console::ClassKey &classKey)
{
    if (classKey.getPackageName() == "org.apache.qpid.broker") {
        const std::string &className = classKey.getClassName();
        if (className == "broker") return Broker;
        if (className == "queue")  return Queue;
        if (className == "system") return System;
    }
    return Other;
}

/**
 * @brief Convert a QMF type code to a human-readable string.
 *
 * This function is only used to make log messages more friendly.
 *
 * @param typeCode QMF type code to convert to string.
 *
 * @return A human-readable version of \a typeCode.
 */
std::string ConsoleUtils::qmfTypeCodeToString(const uint8_t typeCode)
{
    // See Qpid's cpp/include/qmf/engine/Typecode.h
    switch (typeCode) {
        case 1: return "UINT8";
        case 2: return "UINT16";
        case 3: return "UINT32";
        case 4: return "UINT64";
      //case 5: // There is no type 5.
        case 6: return "SSTR";
        case 7: return "LSTR";
        case 8: return "ABSTIME";
        case 9: return "DELTATIME";
        case 10: return "REF";
        case 11: return "BOOL";
        case 12: return "FLOAT";
        case 13: return "DOUBLE";
        case 14: return "UUID";
        case 15: return "MAP";
        case 16: return "INT8";
        case 17: return "INT16";
        case 18: return "INT32";
        case 19: return "INT64";
        case 20: return "OBJECT";
        case 21: return "LIST";
        case 22: return "ARRAY";
        default:
            return "unknown (" + boost::lexical_cast<std::string>(typeCode) + ')';
    }
}

/**
 * @brief Convert a QMF class to a canonical string for logging.
 *
 * @param classKey QMF class key to convert to string.
 *
 * @return String representation of \a classKey, suitable for logging.
 */
std::string ConsoleUtils::toString(const qpid::console::ClassKey &classKey)
{
    return classKey.getPackageName() + ':' + classKey.getClassName();
}

/**
 * @brief Convert a QMF object to a canonical string for logging.
 *
 * @param object             QMF object to conver to string.
 * @param includePackageName If \c true, the object's QMF package name will be
 *                           included in the returned string.
 *
 * @return String representation of \a object, suitable for logging.
 */
std::string ConsoleUtils::toString(const qpid::console::Object &object,
                                   bool includePackageName)
{
    const qpid::console::ClassKey &classKey = object.getClassKey();
    return (includePackageName ? toString(classKey) : classKey.getClassName()) +
           " '" + getName(object) + "' (" + toString(object.getObjectId()) + ')';
}

/**
 * @brief Convert a QMF object ID to a canonical string for logging.
 *
 * @param id QMF object ID to convert to string.
 *
 * @return String representation of \a id, suitable for logging.
 */
std::string ConsoleUtils::toString(const qpid::console::ObjectId &id)
{
    std::ostringstream stream; // Take advantage of the qpid::console::ObjectId
    stream << id;              // class' built-in stream operator.
    return stream.str();
}

/**
 * @brief Convert a QMF property schema to a canonical string for logging.
 *
 * @param property QMF property schema to convert to string.
 *
 * @return String representation of \a property, suitable for logging.
 */
std::string ConsoleUtils::toString(const qpid::console::SchemaProperty &property)
{
    return property.name + ':' + qmfTypeCodeToString(property.typeCode) + ':' +
           property.unit + ':' + property.desc;
}

/**
 * @brief Convert a QMF statistic schema to a canonical string for logging.
 *
 * @param statistic QMF statistic schema to convert to string.
 *
 * @return String representation of \a statistic, suitable for logging.
 */
std::string ConsoleUtils::toString(const qpid::console::SchemaStatistic &statistic)
{
    return statistic.name + ':' + qmfTypeCodeToString(statistic.typeCode) + ':' +
           statistic.unit + ':' + statistic.desc;
}
