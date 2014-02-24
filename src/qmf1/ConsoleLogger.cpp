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

/**
 * @brief Invoked when a connection is established to a broker.
 *
 * @param broker Newly connected broker.
 */
void ConsoleLogger::brokerConnected(const qpid::console::Broker &broker)
{
    __pmNotifyErr(LOG_INFO, "broker %s (%s) connected",
                  broker.getUrl().c_str(), broker.getBrokerId().str().c_str());
}

/**
 * @brief Invoked when the connection to a broker is lost.
 *
 * @param broker Disconnected broker.
 */
void ConsoleLogger::brokerDisconnected(const qpid::console::Broker &broker)
{
    __pmNotifyErr(LOG_INFO, "broker %s (%s) disconnected",
                  broker.getUrl().c_str(), broker.getBrokerId().str().c_str());
}

/**
 * @brief Invoked when a QMF package is discovered.
 *
 * @param package Name of the newly discovered package.
 */
void ConsoleLogger::newPackage(const std::string &package)
{
    if (pmDebug & DBG_TRACE_APPL2) {
        __pmNotifyErr(LOG_DEBUG, "%s %s", __FUNCTION__, package.c_str());
    }
}

/**
 * @brief Invoked when a new class is discovered.
 *
 * @param classKey Key of the newly discovered QMF class.
 */
void ConsoleLogger::newClass(const qpid::console::ClassKey &classKey)
{
    if (pmDebug & DBG_TRACE_APPL2) {
        __pmNotifyErr(LOG_DEBUG, "%s %s", __FUNCTION__,
                      ConsoleUtils::toString(classKey).c_str());
    }
}

/**
 * @brief Invoked when a QMF agent is discovered.
 *
 * @param agent Newly discovered QMF agent.
 */
void ConsoleLogger::newAgent(const qpid::console::Agent &agent)
{
    if (pmDebug & DBG_TRACE_APPL2) {
        __pmNotifyErr(LOG_DEBUG, "%s %s", __FUNCTION__, agent.getLabel().c_str());
    }
}

/**
 * @brief Invoked when a QMF agent disconects.
 *
 * @param agent Disconnected QMF agent.
 */
void ConsoleLogger::delAgent (const qpid::console::Agent &agent)
{
    if (pmDebug & DBG_TRACE_APPL2) {
        __pmNotifyErr(LOG_DEBUG, "%s %s", __FUNCTION__, agent.getLabel().c_str());
    }
}

/**
 * @brief Invoked when an object's propetied ared updated.
 *
 * @param broker Broker advertising the updated object.
 * @param object Updated QMF object.
 */
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

/**
 * @brief Invoked when an object's statistics are updated.
 *
 * @param broker Broker advertising the updated statistics.
 * @param object Updated QMF object.
 */
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

/**
 * @brief Invoked when a QMF event is raised.
 *
 * @param event Raised QMF event.
 */
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

/**
 * @brief Invoked when broker is discovered.
 *
 * @param broker Newly discovered broker.
 */
void ConsoleLogger::brokerInfo(qpid::console::Broker &broker)
{
    if (pmDebug & DBG_TRACE_APPL1) {
        __pmNotifyErr(LOG_DEBUG, "%s %s", __FUNCTION__, broker.getUrl().c_str());
    }
}

/**
 * @brief Log a QMF schema.
 *
 * This function will log (for debugging only) the object's schema, only if this
 * is the first time the schema has been seen.
 *
 * @param object Object from which to fetch the schema to log.
 */
void ConsoleLogger::logSchema(const qpid::console::Object &object)
{
    const qpid::console::SchemaClass * const schemaClass = object.getSchema();
    if (schemaClass != NULL) {
        logSchema(*object.getSchema());
    }
}

/**
 * @brief Log a QMF schema.
 *
 * This function will debug-log the schema, only if this is the first time the
 * schema (as indentified by its class key) has been seen.
 *
 * @param schema QMF schema to log.
 */
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
