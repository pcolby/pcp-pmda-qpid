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
#include <qpid/console/Value.h>

#include <pcp/pmapi.h>
#include <pcp/impl.h>

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
        __pmNotifyErr(LOG_DEBUG, "%s:%d:%s %s", __FILE__, __LINE__, __FUNCTION__,
                      classKey.getClassName().c_str());
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
        message << object.getObjectId();
        const qpid::console::Object::AttributeMap::const_iterator name = object.getAttributes().find("name");
        if (name != object.getAttributes().end()) {
            message << ' ' << name->second->str();
        }
        __pmNotifyErr(LOG_DEBUG, "%s:%d:%s object: %s", __FILE__, __LINE__, __FUNCTION__,
                      message.str().c_str());

        for (qpid::console::Object::AttributeMap::const_iterator attribute = object.getAttributes().begin();
            attribute != object.getAttributes().end(); attribute++) {
            __pmNotifyErr(LOG_DEBUG, "%s:%d:%s   attribute: %s => %s", __FILE__, __LINE__, __FUNCTION__,
                          attribute->first.c_str(), attribute->second->str().c_str());
        }
    }
}

void ConsoleListener::objectStats(qpid::console::Broker &broker, qpid::console::Object &object) {
    /// @todo  This is where it get's interesting.
}

void ConsoleListener::event(qpid::console::Event &event) {
    if (pmDebug & DBG_TRACE_APPL2) {
        __pmNotifyErr(LOG_DEBUG, "%s:%d:%s %s", __FILE__, __LINE__, __FUNCTION__,
                      event.getClassKey().getClassName().c_str());
        for (qpid::console::Object::AttributeMap::const_iterator attribute = event.getAttributes().begin();
             attribute != event.getAttributes().end(); attribute++)
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
