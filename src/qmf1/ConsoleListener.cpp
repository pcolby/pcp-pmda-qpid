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
 * @brief Defines the ConsoleListener class.
 */

#include "ConsoleListener.h"

#include "ConsoleUtils.h"

#include <qpid/console/Value.h>

#include <pcp/pmapi.h>
#include <pcp/impl.h>

/**
 * @brief Default constructor.
 */
ConsoleListener::ConsoleListener() : includeAutoDelete(false)
{

}

/**
 * @brief Get the next new QMF object ID, if any.
 *
 * The ConsoleListener class maintains a list of new object IDs - that is, ones
 * that have not been seen before.  This function can then be used to consume
 * those object IDs one at a time.
 *
 * This allow the owning PMDA instance to check for the arrival of new QMF
 * object, and register them with PCP accordingly.
 *
 * @return A QMF object ID, or an unset boost::optional if no new IDs are
 *         available.
 *
 * @see QpidPmdaQmf1::begin_fetch_values
 */
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

/**
 * @brief Get a QMF properties object for a QMF object ID.
 *
 * @param id QMF object ID to fecth properties for.
 *
 * @return A QMF properties object, or an unset boost::optional if the requested
 *         object ID could not be found in the known properties map.
 */
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

/**
 * @brief Get a QMF statistics object for a QMF object ID.
 *
 * @param id QMF object ID to fecth statistics for.
 *
 * @return A QMF statistics object, or an unset boost::optional if the requested
 *         object ID could not be found in the known statisitics map.
 */
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

/**
 * @brief Set whether or not to track auto-delete objects.
 *
 * Some Qpid object types (particularly queues) can be marked as auto-delete.
 * This is typically done for short-lived, temporary queues, such as internal
 * QMF queues.
 *
 * By default, this class will not track such queues, since doing so can
 * increase, by an couple of orders of magnitude, the number of QMF object to
 * track, and the associated memory.
 *
 * This function may be called to specifiy whether or not such queues are
 * tracked (the QpidPmdaQmf1 class exposes this via the --include-auto-delete
 * command line option).
 *
 * @note Issue #4: Currently this class does not purge old stale QMF objects.
 *       This would rarely be an issue when includeAutoDelete is \c false,
 *       however, you should exercise even greater caution setting
 *       includeAutoDelete to \c true as long as that issues has not been
 *       resolved yet. See https://github.com/pcolby/pcp-pmda-qpid/issues/4
 *
 * @param include Whether or not to include auto-delete objects.
 */
void ConsoleListener::setIncludeAutoDelete(const bool include)
{
    includeAutoDelete = include;
}

/**
 * @brief Invoked when an object's propeties are updated.
 *
 * We override this QMF callback function to record the supplied properties
 * object for any QMF objects of interest (ie those for which isSupported
 * returns \c  true).
 *
 * @param broker Broker advertising the updated object.
 * @param object Updated QMF object.
 *
 * @see isSupported
 */
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

/**
 * @brief Invoked when an object's statistics are updated.
 *
 * We override this QMF callback function to record the supplied statistics
 * object for any QMF objects of interest (ie those for which isSupported
 * returns \c  true).
 *
 * @param broker Broker advertising the updated object.
 * @param object Updated QMF object.
 *
 * @see isSupported
 */
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
            return;
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

/**
 * @brief Is an object marked for auto-deletion?
 *
 * This function checks to see if the given QMF object has set to be auto-
 * deleted.  It does this by looking for a boolean "autoDelete" property set
 * to \c true.
 *
 * @note Since the "autoDelete" property is a QMF property (as opposed to a QMF
 *       statistic), this function will only work for QMF property object, not
 *       QMF statistics objects - for the latter, it will always return \c false
 *       as statisitics objects do not (normally) contain autoDelete attributes.
 *
 * @param object QMF properties object to check for auto-delete status.
 *
 * @return \c true if \a object has a boolean autoDelete property set to \c true.
 */
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

/**
 * @brief Are objects of the given \c classKey supported by this PMDA.
 *
 * Currently, this function assumes that all defined
 * ConsoleUtils::ObjectSchemaType types are supported by this PMDA, except for
 * ConsoleUtils::Other.
 *
 * @param classKey QMF class key to check for support.
 *
 * @return \c true if objects of type \c classKey are supported by this PMDA.
 *
 * @see ConsoleUtils::ObjectSchemaType
 */
bool ConsoleListener::isSupported(const qpid::console::ClassKey &classKey)
{
    return (ConsoleUtils::getType(classKey) != ConsoleUtils::Other);
}
