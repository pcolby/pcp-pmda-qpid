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

#include "ConsoleUtils.h"

#include <qpid/console/Value.h>

#include <boost/lexical_cast.hpp>

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

ConsoleUtils::ObjectSchemaType ConsoleUtils::getType(const qpid::console::Object &object)
{
    return getType(object.getClassKey());
}

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

std::string ConsoleUtils::toString(const qpid::console::ClassKey &classKey)
{
    return classKey.getPackageName() + ':' + classKey.getClassName();
}

std::string ConsoleUtils::toString(const qpid::console::Object &object,
                                   bool includePackageName)
{
    const qpid::console::ClassKey &classKey = object.getClassKey();
    return (includePackageName ? toString(classKey) : classKey.getClassName()) +
           " '" + getName(object) + "' (" + toString(object.getObjectId()) + ')';
}

std::string ConsoleUtils::toString(const qpid::console::ObjectId &id)
{
    std::ostringstream stream;
    stream << id;
    return stream.str();
}

std::string ConsoleUtils::toString(const qpid::console::SchemaProperty &property)
{
    return property.name + ':' + qmfTypeCodeToString(property.typeCode) + ':' +
           property.unit + ':' + property.desc;
}

std::string ConsoleUtils::toString(const qpid::console::SchemaStatistic &statistic)
{
    return statistic.name + ':' + qmfTypeCodeToString(statistic.typeCode) + ':' +
           statistic.unit + ':' + statistic.desc;
}
