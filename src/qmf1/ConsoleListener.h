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

#ifndef __QPID_PMDA_CONSOLE_LISTENER_H__
#define __QPID_PMDA_CONSOLE_LISTENER_H__

#include "ConsoleLogger.h"

#include <boost/optional/optional.hpp>
#include <boost/thread/mutex.hpp>

#include <queue>

class ConsoleListener : public ConsoleLogger {

public:
    ConsoleListener();

    boost::optional<qpid::console::ObjectId> getNewObjectId();

    boost::optional<qpid::console::Object> getProps(const qpid::console::ObjectId &id);

    boost::optional<qpid::console::Object> getStats(const qpid::console::ObjectId &id);

    void setIncludeAutoDelete(const bool include = true);

    /* Overrides for qpid::console::ConsoleListener events below here */

    virtual void objectProps(qpid::console::Broker &broker, qpid::console::Object &object);

    virtual void objectStats(qpid::console::Broker &broker, qpid::console::Object &object);

protected:
    bool includeAutoDelete;

    virtual bool isAutoDelete(const qpid::console::Object &object);

    virtual bool isSupported(const qpid::console::ClassKey &classKey);

private:
    typedef std::map<qpid::console::ObjectId, qpid::console::Object> ObjectMap;

    ObjectMap props, stats;
    boost::mutex propsMutex, statsMutex;

    std::queue<qpid::console::ObjectId> newObjects;
    boost::mutex newObjectsMutex;

};

#endif
