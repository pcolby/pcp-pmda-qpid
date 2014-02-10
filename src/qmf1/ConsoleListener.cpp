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

#include <qpid/console/Value.h>

#include <pcp/pmapi.h>
#include <pcp/impl.h>

ConsoleListener::ConsoleListener() : includeAutoDelete(false)
{

}

boost::optional<qpid::console::ObjectId> ConsoleListener::getNewObjectId()
{
    boost::optional<qpid::console::ObjectId> id;
    boost::unique_lock<boost::mutex> lock(newObjectsMutex);
    if (!newObjects.empty()) {
        id = newObjects.front();
        newObjects.pop();
    }
    return id;
}

boost::optional<qpid::console::Object> ConsoleListener::getProps(const qpid::console::ObjectId &id)
{
    boost::optional<qpid::console::Object> object;
    boost::unique_lock<boost::mutex> lock(propsMutex);
    const ObjectMap::const_iterator iter = props.find(id);
    if (iter != props.end()) {
        object = iter->second;
    }
    return object;
}

boost::optional<qpid::console::Object> ConsoleListener::getStats(const qpid::console::ObjectId &id)
{
    boost::optional<qpid::console::Object> object;
    boost::unique_lock<boost::mutex> lock(statsMutex);
    const ObjectMap::const_iterator iter = stats.find(id);
    if (iter != stats.end()) {
        object = iter->second;
    }
    return object;
}

void ConsoleListener::setIncludeAutoDelete(const bool include)
{
    includeAutoDelete = include;
}

void ConsoleListener::objectProps(qpid::console::Broker &broker,
                                  qpid::console::Object &object)
{
    // Let the super implementation log the properties.
    ConsoleLogger::objectProps(broker, object);

    // Skip unsupported object types.
    if (!isSupported(object.getClassKey())) {
        return;
    }

    // Skip autoDel queues, unless includeAutoDelete is set.
    if ((!includeAutoDelete) && (isAutoDelete(object))) {
        return;
    }

    // Save the properties for future fetch metrics requests.
    boost::unique_lock<boost::mutex> lock(propsMutex);
    const ObjectMap::iterator iter = props.find(object.getObjectId());
    if (iter == props.end()) {
        props.insert(std::make_pair(object.getObjectId(), object));
        __pmNotifyErr(LOG_INFO, "new %s", ConsoleUtils::toString(object).c_str());
        boost::unique_lock<boost::mutex> lock(newObjectsMutex);
        newObjects.push(object.getObjectId());
    } else {
        iter->second = object;
    }
}

void ConsoleListener::objectStats(qpid::console::Broker &broker,
                                  qpid::console::Object &object)
{
    // Let the super implementation log the properties.
    ConsoleLogger::objectStats(broker, object);

    // Skip unsupported object types.
    if (!isSupported(object.getClassKey())) {
        return;
    }

    // Skip autoDel queues, unless includeAutoDelete is set.
    if (!includeAutoDelete) {
        // We need the props object (not stats) to determine the autoDel status.
        boost::unique_lock<boost::mutex> lock(propsMutex);
        const ObjectMap::const_iterator iter = props.find(object.getObjectId());
        if (iter == props.end()) {
            if (pmDebug & DBG_TRACE_APPL1) {
                // This happens because objectProps above, skipped this object appropriately.
                __pmNotifyErr(LOG_DEBUG, "ignoring statistics for %s since we have no properties",
                              ConsoleUtils::toString(object).c_str());
            }
        } else if (isAutoDelete(iter->second)) {
            return;
        }
    }

    // Save the statistics for future fetch metrics requests.
    boost::unique_lock<boost::mutex> lock(statsMutex);
    const ObjectMap::iterator iter = stats.find(object.getObjectId());
    if (iter == stats.end()) {
        stats.insert(std::make_pair(object.getObjectId(), object));
    } else {
        iter->second = object;
    }
}

bool ConsoleListener::isAutoDelete(const qpid::console::Object &object)
{
    const qpid::console::Object::AttributeMap &attributes = object.getAttributes();
    const qpid::console::Object::AttributeMap::const_iterator autoDelete = attributes.find("autoDelete");

    if (autoDelete == attributes.end()) {
        if (pmDebug & DBG_TRACE_APPL1) {
            __pmNotifyErr(LOG_DEBUG, "%s has no autoDelete property",
                          ConsoleUtils::toString(object).c_str());
        }
        return false;
    }

    if (!autoDelete->second->isBool()) {
        __pmNotifyErr(LOG_NOTICE, "autoDelete property for %s is not a boolean",
                      ConsoleUtils::toString(object).c_str());
        return false;
    }

    if (pmDebug & DBG_TRACE_APPL2) {
        __pmNotifyErr(LOG_DEBUG, "%s autoDelete: %s",
                      ConsoleUtils::toString(object).c_str(),
                      autoDelete->second->asBool() ? "true" : "false");
    }

    return autoDelete->second->asBool();
}

bool ConsoleListener::isSupported(const qpid::console::ClassKey &classKey)
{
    return (ConsoleUtils::getType(classKey) != ConsoleUtils::Other);
}
