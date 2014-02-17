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

#include "QpidLogger.h"

#include <pcp/pmapi.h>
#include <pcp/impl.h>

#include <boost/algorithm/string/trim.hpp>

void QpidLogger::log(const qpid::log::Statement &statement,
                     const std::string &message)
{
    const int level = pmNotifyLevel(statement.level);

    if ((statement.level == qpid::log::trace) && !(pmDebug & DBG_TRACE_APPL2)) {
        return;
    }

    if ((statement.level == qpid::log::debug) && !(pmDebug & DBG_TRACE_APPL1)) {
        return;
    }

    __pmNotifyErr(level, "QpidLogger: %s", boost::trim_copy(message).c_str());
}

int QpidLogger::pmNotifyLevel(const qpid::log::Level level)
{
    switch (level) {
        case qpid::log::trace:    return LOG_DEBUG;
        case qpid::log::debug:    return LOG_DEBUG;
        case qpid::log::info:     return LOG_INFO;
        case qpid::log::notice:   return LOG_NOTICE;
        case qpid::log::warning:  return LOG_WARNING;
        case qpid::log::error:    return LOG_ERR;
        case qpid::log::critical: return LOG_CRIT;
        default:
            __pmNotifyErr(LOG_NOTICE, "unknown Qpid log level %d", level);
            return LOG_ERR;
    }
}
