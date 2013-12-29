/*
 * Copyright 2013 Paul Colby
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

#include <pcp/pmapi.h>
#include <pcp/impl.h>

void ConsoleListener::brokerConnected(const qpid::console::Broker &broker) {
    __pmNotifyErr(LOG_DEBUG, "broker %s (%s) connected",
                  broker.getUrl().c_str(), broker.getBrokerId().str().c_str());
}

void ConsoleListener::brokerDisconnected(const qpid::console::Broker &broker) {
    __pmNotifyErr(LOG_DEBUG, "broker %s (%s) disconnected",
                  broker.getUrl().c_str(), broker.getBrokerId().str().c_str());
}

void ConsoleListener::newPackage(const std::string &package) {
    //std::cout << "package: " << package << std::endl;
}

void ConsoleListener::newClass(const qpid::console::ClassKey &classKey) {
    //std::cout << "class: " << classKey.getClassName() << std::endl;
}

void ConsoleListener::newAgent(const qpid::console::Agent &agent) {
    //std::cout << "newAgent: " << agent.getLabel() << std::endl;
}

void ConsoleListener::delAgent (const qpid::console::Agent &agent) {
    //std::cout << "delAgent: " << agent.getLabel() << std::endl;
}

void ConsoleListener::objectProps(qpid::console::Broker &broker, qpid::console::Object &object) {
    //std::cout << "object: " << object.getObjectId();
    //const Object::AttributeMap::const_iterator name = object.getAttributes().find("name");
    //if (name != object.getAttributes().end())
        //std::cout << ' ' << name->second->str();
    //std::cout << std::endl;

    //for (Object::AttributeMap::const_iterator attribute = object.getAttributes().begin();
        //attribute != object.getAttributes().end(); attribute++) {
        //std::cout << "  attribute: " << attribute->first << " => "
          //        << attribute->second->str() << std::endl;
    //}
}

void ConsoleListener::objectStats(qpid::console::Broker &broker, qpid::console::Object &object) {
}

void ConsoleListener::event(qpid::console::Event &event) {
    //std::cout << "event: " << event.getClassKey().getClassName() << std::endl;
    //for (Object::AttributeMap::const_iterator attribute = event.getAttributes().begin();
        //attribute != event.getAttributes().end(); attribute++) {
        //std::cout << "  attribute: " << attribute->first << " => "
          //        << attribute->second->str() << std::endl;
    //}
}

void ConsoleListener::brokerInfo(qpid::console::Broker &broker) {
    //std::cout << "brokerInfo: " << broker.getUrl() << std::endl;
}
