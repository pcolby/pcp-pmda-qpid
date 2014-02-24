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

/**
 * @file
 * @brief Defines the ConsoleLogger class.
 */

#include "ConsoleLogger.h"

#include "ConsoleUtils.h"

#include <qpid/console/Agent.h>
#include <qpid/console/Object.h>
#include <qpid/console/Schema.h>
#include <qpid/console/Value.h>

#include <pcp/pmapi.h>
#include <pcp/impl.h>

void ConsoleLogger::brokerConnected(const qpid::console::Broker &broker)
{
    __pmNotifyErr(LOG_INFO, "broker %s (%s) connected",
                  broker.getUrl().c_str(), broker.getBrokerId().str().c_str());
}

void ConsoleLogger::brokerDisconnected(const qpid::console::Broker &broker)
{
    __pmNotifyErr(LOG_INFO, "broker %s (%s) disconnected",
                  broker.getUrl().c_str(), broker.getBrokerId().str().c_str());
}

void ConsoleLogger::newPackage(const std::string &package)
{
    if (pmDebug & DBG_TRACE_APPL2) {
        __pmNotifyErr(LOG_DEBUG, "%s %s", __FUNCTION__, package.c_str());
    }
}

void ConsoleLogger::newClass(const qpid::console::ClassKey &classKey)
{
    if (pmDebug & DBG_TRACE_APPL2) {
        __pmNotifyErr(LOG_DEBUG, "%s %s", __FUNCTION__,
                      ConsoleUtils::toString(classKey).c_str());
    }
}

void ConsoleLogger::newAgent(const qpid::console::Agent &agent)
{
    if (pmDebug & DBG_TRACE_APPL2) {
        __pmNotifyErr(LOG_DEBUG, "%s %s", __FUNCTION__, agent.getLabel().c_str());
    }
}

void ConsoleLogger::delAgent (const qpid::console::Agent &agent)
{
    if (pmDebug & DBG_TRACE_APPL2) {
        __pmNotifyErr(LOG_DEBUG, "%s %s", __FUNCTION__, agent.getLabel().c_str());
    }
}

void ConsoleLogger::objectProps(qpid::console::Broker &/*broker*/,
                                  qpid::console::Object &object)
{
    if (pmDebug & DBG_TRACE_APPL2) {
        __pmNotifyErr(LOG_DEBUG, "%s object: %s", __FUNCTION__,
                      ConsoleUtils::toString(object, true).c_str());

        logSchema(object);

        for (qpid::console::Object::AttributeMap::const_iterator attribute = object.getAttributes().begin();
            attribute != object.getAttributes().end(); ++attribute) {
            __pmNotifyErr(LOG_DEBUG, "%s   attribute: %s => %s", __FUNCTION__,
                          attribute->first.c_str(), attribute->second->str().c_str());
        }
    }
}

void ConsoleLogger::objectStats(qpid::console::Broker &/*broker*/,
                                  qpid::console::Object &object)
{
    if (pmDebug & DBG_TRACE_APPL2) {
        __pmNotifyErr(LOG_DEBUG, "%s object: %s", __FUNCTION__,
                      ConsoleUtils::toString(object, true).c_str());

        logSchema(object);

        for (qpid::console::Object::AttributeMap::const_iterator attribute = object.getAttributes().begin();
            attribute != object.getAttributes().end(); ++attribute) {
            __pmNotifyErr(LOG_DEBUG, "%s   attribute: %s => %s", __FUNCTION__,
                          attribute->first.c_str(), attribute->second->str().c_str());
        }
    }
}

void ConsoleLogger::event(qpid::console::Event &event)
{
    if (pmDebug & DBG_TRACE_APPL2) {
        __pmNotifyErr(LOG_DEBUG, "%s %s", __FUNCTION__,
                      event.getClassKey().getClassName().c_str());
        for (qpid::console::Object::AttributeMap::const_iterator attribute = event.getAttributes().begin();
             attribute != event.getAttributes().end(); ++attribute)
        {
            __pmNotifyErr(LOG_DEBUG, "%s   attribute: %s => %s", __FUNCTION__,
                          attribute->first.c_str(), attribute->second->str().c_str());
        }
    }
}

void ConsoleLogger::brokerInfo(qpid::console::Broker &broker)
{
    if (pmDebug & DBG_TRACE_APPL1) {
        __pmNotifyErr(LOG_DEBUG, "%s %s", __FUNCTION__, broker.getUrl().c_str());
    }
}

void ConsoleLogger::logSchema(const qpid::console::Object &object)
{
    const qpid::console::SchemaClass * const schemaClass = object.getSchema();
    if (schemaClass != NULL) {
        logSchema(*object.getSchema());
    }
}

void ConsoleLogger::logSchema(const qpid::console::SchemaClass &schema)
{
    static std::set<std::string> seenAlready;
    if ((pmDebug & DBG_TRACE_APPL2) && (seenAlready.count(schema.getClassKey().str()) == 0)) {
        __pmNotifyErr(LOG_DEBUG, "%s %s", __FUNCTION__,
                      ConsoleUtils::toString(schema.getClassKey()).c_str());

        for (std::vector<qpid::console::SchemaProperty *>::const_iterator property = schema.properties.begin();
            property != schema.properties.end(); ++property) {
            __pmNotifyErr(LOG_DEBUG, "%s   property: %s", __FUNCTION__,
                          ConsoleUtils::toString(**property).c_str());
        }

        for (std::vector<qpid::console::SchemaStatistic *>::const_iterator statistic = schema.statistics.begin();
            statistic != schema.statistics.end(); ++statistic) {
            __pmNotifyErr(LOG_DEBUG, "%s   statistic: %s", __FUNCTION__,
                          ConsoleUtils::toString(**statistic).c_str());
        }
        seenAlready.insert(schema.getClassKey().str());
    }
}
