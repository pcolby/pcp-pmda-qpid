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
 * @brief Defines the Qpid PMDA entry point.
 */

#include "QpidLogger.h"

#include "qmf1/QpidPmdaQmf1.h"

/// Single shared instance for both QMFv1 and QMFv2 PMDAs.
pcp::pmda * pcp::pmda::instance(NULL);

/**
 * @brief Qpid PMDA main entry point.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 *
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error.
 */
int main(int argc, char *argv[]) {
    using namespace qpid::log;
    Logger::instance().format(0); // Don't log timestamps, etc, since PCP will.
    Logger::instance().output(std::auto_ptr<Logger::Output>(new QpidLogger));
    const int result = pcp::pmda::run_daemon<QpidPmdaQmf1>(argc, argv);
    Logger::instance().clear();
    return result;
}
