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

#include "ConsoleListener.hpp"

#include <qpid/console/Agent.h>
#include <qpid/console/Object.h>
#include <qpid/console/Schema.h>
#include <qpid/console/Value.h>

#include <pcp/pmapi.h>
#include <pcp/impl.h>

#include <boost/lexical_cast.hpp>

void ConsoleListener::brokerConnected(const qpid::console::Broker &broker) {
    __pmNotifyErr(LOG_INFO, "broker %s (%s) connected",
                  broker.getUrl().c_str(), broker.getBrokerId().str().c_str());
}

void ConsoleListener::brokerDisconnected(const qpid::console::Broker &broker) {
    __pmNotifyErr(LOG_INFO, "broker %s (%s) disconnected",
                  broker.getUrl().c_str(), broker.getBrokerId().str().c_str());
}

void ConsoleListener::newPackage(const std::string &package) {
    if (pmDebug & DBG_TRACE_APPL2) {
        __pmNotifyErr(LOG_DEBUG, "%s:%d:%s %s", __FILE__, __LINE__, __FUNCTION__,
                      package.c_str());
    }
}

void ConsoleListener::newClass(const qpid::console::ClassKey &classKey) {
    if (pmDebug & DBG_TRACE_APPL2) {
        __pmNotifyErr(LOG_DEBUG, "%s:%d:%s %s:%s", __FILE__, __LINE__, __FUNCTION__,
                      classKey.getPackageName().c_str(), classKey.getClassName().c_str());
    }
}

void ConsoleListener::newAgent(const qpid::console::Agent &agent) {
    if (pmDebug & DBG_TRACE_APPL2) {
        __pmNotifyErr(LOG_DEBUG, "%s:%d:%s %s", __FILE__, __LINE__, __FUNCTION__,
                      agent.getLabel().c_str());
    }
}

void ConsoleListener::delAgent (const qpid::console::Agent &agent) {
    if (pmDebug & DBG_TRACE_APPL2) {
        __pmNotifyErr(LOG_DEBUG, "%s:%d:%s %s", __FILE__, __LINE__, __FUNCTION__,
                      agent.getLabel().c_str());
    }
}

void ConsoleListener::objectProps(qpid::console::Broker &broker, qpid::console::Object &object) {
    if (pmDebug & DBG_TRACE_APPL2) {
        std::ostringstream message;
        message << object.getClassKey().getPackageName() << ':'
                << object.getClassKey().getClassName() << ':' << object.getObjectId();
        const qpid::console::Object::AttributeMap::const_iterator name = object.getAttributes().find("name");
        if (name != object.getAttributes().end()) {
            message << ' ' << name->second->str();
        }
        __pmNotifyErr(LOG_DEBUG, "%s:%d:%s object: %s", __FILE__, __LINE__, __FUNCTION__,
                      message.str().c_str());

        logSchema(object);

        for (qpid::console::Object::AttributeMap::const_iterator attribute = object.getAttributes().begin();
            attribute != object.getAttributes().end(); ++attribute) {
            __pmNotifyErr(LOG_DEBUG, "%s:%d:%s   attribute: %s => %s", __FILE__, __LINE__, __FUNCTION__,
                          attribute->first.c_str(), attribute->second->str().c_str());
        }
    }
}

void ConsoleListener::objectStats(qpid::console::Broker &broker, qpid::console::Object &object) {
    const bool isQueue = (object.getSchema()->key.getPackageName() == "org.apache.qpid.broker" &&
                          object.getSchema()->key.getClassName() == "queue");

    if (pmDebug & (isQueue ? DBG_TRACE_APPL1 : DBG_TRACE_APPL2)) {
        std::ostringstream message;
        message << object.getSchema()->key.getPackageName() << ':' << object.getSchema()->key.getClassName();
        const qpid::console::Object::AttributeMap::const_iterator name = object.getAttributes().find("name");
        if (name != object.getAttributes().end()) {
            message << ' ' << name->second->str();
        }
        __pmNotifyErr(LOG_DEBUG, "%s:%d:%s object: %s", __FILE__, __LINE__, __FUNCTION__,
                      message.str().c_str());

        logSchema(object);

        for (qpid::console::Object::AttributeMap::const_iterator attribute = object.getAttributes().begin();
            attribute != object.getAttributes().end(); ++attribute) {
            __pmNotifyErr(LOG_DEBUG, "%s:%d:%s   attribute: %s => %s", __FILE__, __LINE__, __FUNCTION__,
                          attribute->first.c_str(), attribute->second->str().c_str());
        }
    }

    // Only interested in queue's from here on.
    if (!isQueue) {
        return;
    }

    /// @todo Record stats :)
}

void ConsoleListener::event(qpid::console::Event &event) {
    if (pmDebug & DBG_TRACE_APPL2) {
        __pmNotifyErr(LOG_DEBUG, "%s:%d:%s %s", __FILE__, __LINE__, __FUNCTION__,
                      event.getClassKey().getClassName().c_str());
        for (qpid::console::Object::AttributeMap::const_iterator attribute = event.getAttributes().begin();
             attribute != event.getAttributes().end(); ++attribute)
        {
            __pmNotifyErr(LOG_DEBUG, "%s:%d:%s   attribute: %s => %s", __FILE__, __LINE__, __FUNCTION__,
                          attribute->first.c_str(), attribute->second->str().c_str());
        }
    }
}

void ConsoleListener::brokerInfo(qpid::console::Broker &broker) {
    if (pmDebug & DBG_TRACE_APPL1) {
        __pmNotifyErr(LOG_DEBUG, "%s:%d:%s %s", __FILE__, __LINE__, __FUNCTION__,
                      broker.getUrl().c_str());
    }
}

void ConsoleListener::logSchema(const qpid::console::Object &object) {
    logSchema(*object.getSchema());
}

// Just for debugging.
void ConsoleListener::logSchema(const qpid::console::SchemaClass &schema) {
    static std::set<std::string> seenAlready;
    if (seenAlready.count(schema.getClassKey().str()) == 0) {
        __pmNotifyErr(LOG_DEBUG, "%s:%d:%s %s:%s", __FILE__, __LINE__, __FUNCTION__,
                      schema.getClassKey().getPackageName().c_str(),
                      schema.getClassKey().getClassName().c_str());

        for (std::vector<qpid::console::SchemaProperty *>::const_iterator property = schema.properties.begin();
            property != schema.properties.end(); ++property) {
            __pmNotifyErr(LOG_DEBUG, "%s:%d:%s   property: %s:%s:%s:%s", __FILE__, __LINE__, __FUNCTION__,
                          (*property)->name.c_str(), qmfTypeCodeToString((*property)->typeCode).c_str(),
                          (*property)->unit.c_str(), (*property)->desc.c_str());
        }

        for (std::vector<qpid::console::SchemaStatistic *>::const_iterator statistic = schema.statistics.begin();
            statistic != schema.statistics.end(); ++statistic) {
            __pmNotifyErr(LOG_DEBUG, "%s:%d:%s   statistic: %s:%s:%s:%s", __FILE__, __LINE__, __FUNCTION__,
                          (*statistic)->name.c_str(), qmfTypeCodeToString((*statistic)->typeCode).c_str(),
                          (*statistic)->unit.c_str(), (*statistic)->desc.c_str());
        }
        seenAlready.insert(schema.getClassKey().str());
    }
}

std::string ConsoleListener::qmfTypeCodeToString(const uint8_t typeCode) {
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
