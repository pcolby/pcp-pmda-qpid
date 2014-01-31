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

class ConsoleListener : public qpid::console::ConsoleListener {

public:

    // Overrides for qpid::console::ConsoleListener events.

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
    virtual std::string qmfTypeCodeToString(const uint8_t typeCode);

};
