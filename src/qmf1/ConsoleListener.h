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

#include <qpid/console/ConsoleListener.h>

#include <boost/optional/optional.hpp>
#include <boost/thread/mutex.hpp>

#include <queue>

class ConsoleListener : public qpid::console::ConsoleListener {

public:

    boost::optional<qpid::console::ObjectId> getNewObjectId();

    boost::optional<qpid::console::Object> getProps(const qpid::console::ObjectId &id);

    boost::optional<qpid::console::Object> getStats(const qpid::console::ObjectId &id);

    /* Overrides for qpid::console::ConsoleListener events below here */

    virtual void brokerConnected(const qpid::console::Broker &broker);

    virtual void brokerDisconnected(const qpid::console::Broker &broker);

    virtual void newPackage(const std::string &package);

    virtual void newClass(const qpid::console::ClassKey &classKey);

    virtual void newAgent(const qpid::console::Agent &agent);

    virtual void delAgent (const qpid::console::Agent &agent);

    virtual void objectProps(qpid::console::Broker &broker, qpid::console::Object &object);

    virtual void objectStats(qpid::console::Broker &broker, qpid::console::Object &object);

    virtual void event(qpid::console::Event &event);

    virtual void brokerInfo(qpid::console::Broker &broker);

protected:
    virtual bool isSupported(const qpid::console::Object &object);

    virtual bool isSupported(const qpid::console::SchemaClass &schemaClass);

    virtual bool isSupported(const qpid::console::ClassKey &classKey);

    virtual void logSchema(const qpid::console::Object &object);

    virtual void logSchema(const qpid::console::SchemaClass &schema);

private:
    typedef std::map<qpid::console::ObjectId, qpid::console::Object> ObjectMap;

    ObjectMap props, stats;
    boost::mutex propsMutex, statsMutex;

    std::queue<qpid::console::ObjectId> newObjects;
    boost::mutex newObjectsMutex;

};
