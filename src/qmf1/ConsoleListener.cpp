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

#include "ConsoleListener.h"

#include "ConsoleUtils.h"

#include <qpid/console/Agent.h>
#include <qpid/console/Object.h>
#include <qpid/console/Schema.h>
#include <qpid/console/Value.h>

#include <pcp/pmapi.h>
#include <pcp/impl.h>

boost::optional<qpid::console::ObjectId> ConsoleListener::getNewObjectId() {
    boost::optional<qpid::console::ObjectId> id;
    boost::unique_lock<boost::mutex> lock(newObjectsMutex);
    if (!newObjects.empty()) {
        id = newObjects.front();
        newObjects.pop();
    }
    return id;
}

boost::optional<qpid::console::Object> ConsoleListener::getProps(const qpid::console::ObjectId &id) {
    boost::optional<qpid::console::Object> object;
    boost::unique_lock<boost::mutex> lock(propsMutex);
    const ObjectMap::const_iterator iter = props.find(id);
    if (iter != props.end()) {
        object = iter->second;
    }
    return object;
}

boost::optional<qpid::console::Object> ConsoleListener::getStats(const qpid::console::ObjectId &id) {
    boost::optional<qpid::console::Object> object;
    boost::unique_lock<boost::mutex> lock(statsMutex);
    const ObjectMap::const_iterator iter = stats.find(id);
    if (iter != stats.end()) {
        object = iter->second;
    }
    return object;
}

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

void ConsoleListener::objectProps(qpid::console::Broker &/*broker*/, qpid::console::Object &object) {
    const bool supported = isSupported(object.getClassKey());

    if (pmDebug & (supported ? DBG_TRACE_APPL1 : DBG_TRACE_APPL2)) {
        __pmNotifyErr(LOG_DEBUG, "%s:%d:%s object: %s", __FILE__, __LINE__, __FUNCTION__,
                      ConsoleUtils::toString(object, true).c_str());

        logSchema(object);

        for (qpid::console::Object::AttributeMap::const_iterator attribute = object.getAttributes().begin();
            attribute != object.getAttributes().end(); ++attribute) {
            __pmNotifyErr(LOG_DEBUG, "%s:%d:%s   attribute: %s => %s", __FILE__, __LINE__, __FUNCTION__,
                          attribute->first.c_str(), attribute->second->str().c_str());
        }
    }

    if (supported) {
        boost::unique_lock<boost::mutex> lock(propsMutex);
        const ObjectMap::iterator iter = props.find(object.getObjectId());
        if (iter == props.end()) {
            props.insert(std::make_pair(object.getObjectId(), object));
            boost::unique_lock<boost::mutex> lock(statsMutex);
            if (stats.find(object.getObjectId()) == stats.end()) {
                std::ostringstream stream;
                stream << object.getObjectId();
                __pmNotifyErr(LOG_INFO, "new props: %s", stream.str().c_str());
                boost::unique_lock<boost::mutex> lock(newObjectsMutex);
                newObjects.push(object.getObjectId());
            }
        } else {
            iter->second = object;
        }
    }
}

void ConsoleListener::objectStats(qpid::console::Broker &/*broker*/, qpid::console::Object &object) {
    const bool supported = isSupported(object.getClassKey());

    if (pmDebug & (supported ? DBG_TRACE_APPL1 : DBG_TRACE_APPL2)) {
        __pmNotifyErr(LOG_DEBUG, "%s:%d:%s object: %s", __FILE__, __LINE__, __FUNCTION__,
                      ConsoleUtils::toString(object, true).c_str());

        logSchema(object);

        for (qpid::console::Object::AttributeMap::const_iterator attribute = object.getAttributes().begin();
            attribute != object.getAttributes().end(); ++attribute) {
            __pmNotifyErr(LOG_DEBUG, "%s:%d:%s   attribute: %s => %s", __FILE__, __LINE__, __FUNCTION__,
                          attribute->first.c_str(), attribute->second->str().c_str());
        }
    }

    if (supported) {
        boost::unique_lock<boost::mutex> lock(statsMutex);
        const ObjectMap::iterator iter = stats.find(object.getObjectId());
        if (iter == stats.end()) {
            stats.insert(std::make_pair(object.getObjectId(), object));
            boost::unique_lock<boost::mutex> lock(propsMutex);
            if (props.find(object.getObjectId()) == props.end()) {
                std::ostringstream stream;
                stream << object.getObjectId();
                __pmNotifyErr(LOG_INFO, "new stats: %s", stream.str().c_str());
                boost::unique_lock<boost::mutex> lock(newObjectsMutex);
                newObjects.push(object.getObjectId());
            }
        } else {
            iter->second = object;
        }
    }
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

bool ConsoleListener::isSupported(const qpid::console::ClassKey &classKey) {
    return (ConsoleUtils::getType(classKey) != ConsoleUtils::Other);
}

void ConsoleListener::logSchema(const qpid::console::Object &object) {
    const qpid::console::SchemaClass * const schemaClass = object.getSchema();
    if (schemaClass != NULL) {
        logSchema(*object.getSchema());
    }
}

// Just for debugging.
void ConsoleListener::logSchema(const qpid::console::SchemaClass &schema) {
    static std::set<std::string> seenAlready;
    if ((pmDebug & DBG_TRACE_APPL2) && (seenAlready.count(schema.getClassKey().str()) == 0)) {
        __pmNotifyErr(LOG_DEBUG, "%s:%d:%s %s", __FILE__, __LINE__, __FUNCTION__,
                      ConsoleUtils::toString(schema.getClassKey()).c_str());

        for (std::vector<qpid::console::SchemaProperty *>::const_iterator property = schema.properties.begin();
            property != schema.properties.end(); ++property) {
            __pmNotifyErr(LOG_DEBUG, "%s:%d:%s   property: %s:%s:%s:%s", __FILE__, __LINE__, __FUNCTION__,
                          (*property)->name.c_str(), ConsoleUtils::qmfTypeCodeToString((*property)->typeCode).c_str(),
                          (*property)->unit.c_str(), (*property)->desc.c_str());
        }

        for (std::vector<qpid::console::SchemaStatistic *>::const_iterator statistic = schema.statistics.begin();
            statistic != schema.statistics.end(); ++statistic) {
            __pmNotifyErr(LOG_DEBUG, "%s:%d:%s   statistic: %s:%s:%s:%s", __FILE__, __LINE__, __FUNCTION__,
                          (*statistic)->name.c_str(), ConsoleUtils::qmfTypeCodeToString((*statistic)->typeCode).c_str(),
                          (*statistic)->unit.c_str(), (*statistic)->desc.c_str());
        }
        seenAlready.insert(schema.getClassKey().str());
    }
}
