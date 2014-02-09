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

#include <pcp/pmapi.h>
#include <pcp/impl.h>

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

void ConsoleListener::objectProps(qpid::console::Broker &broker,
                                  qpid::console::Object &object)
{
    const bool supported = isSupported(object.getClassKey());

    ConsoleLogger::objectProps(broker, object);

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

void ConsoleListener::objectStats(qpid::console::Broker &broker,
                                  qpid::console::Object &object)
{
    const bool supported = isSupported(object.getClassKey());

    ConsoleLogger::objectStats(broker, object);

    if (supported) {
        boost::unique_lock<boost::mutex> lock(statsMutex);
        const ObjectMap::iterator iter = stats.find(object.getObjectId());
        if (iter == stats.end()) {
            stats.insert(std::make_pair(object.getObjectId(), object));
        } else {
            iter->second = object;
        }
    }
}

bool ConsoleListener::isSupported(const qpid::console::ClassKey &classKey)
{
    return (ConsoleUtils::getType(classKey) != ConsoleUtils::Other);
}
