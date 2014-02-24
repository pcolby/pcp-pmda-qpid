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
 * @brief Declares the QpidPmdaQmf1 class.
 */

#ifndef __QPID_PMDA_QMF1_H__
#define __QPID_PMDA_QMF1_H__

// Since we're potentially including two separate PMDAs (this one, for QMFv1,
// and another for QMFv2), tell the header-only PMDA++ library to skip defining
// the static pcp::pmda::instance variable. The main qpidpmda.cpp file will
// define it once for both PMDA classes.
#define PCP_CPP_SKIP_PCP_PMDA_INSTANCE_DEFINITION

#include <pcp-cpp/instance_domain.hpp>
#include <pcp-cpp/pmda.hpp>

#include <qpid/client/ConnectionSettings.h>
#include <qpid/console/SessionManager.h>

#include "ConsoleListener.h"

/**
 * @brief Qpid PMDA using QMF version 1.
 */
class QpidPmdaQmf1 : public pcp::pmda {

public:
    QpidPmdaQmf1();

    virtual std::string get_pmda_name() const;

    virtual int get_default_pmda_domain_number() const;

    virtual std::string get_pmda_version() const;

protected:
    bool nonPmdaMode; ///< Was "non-pmda" mode requested (on the command line).

    /// A simple vector of QMF console connections to establish.
    std::vector<qpid::client::ConnectionSettings> qpidConnectionSettings;

    pcp::instance_domain broker_domain; ///< The "broker" instance domain.
    pcp::instance_domain queue_domain;  ///< The "queue" instance domain.
    pcp::instance_domain system_domain; ///< The "system" instance domain.

    ConsoleListener consoleListener;              ///< A QMF console listener.
    qpid::console::SessionManager sessionManager; ///< A QMF session manager.

    virtual boost::program_options::options_description get_supported_options() const;

    virtual boost::program_options::options_description get_supported_hidden_options() const;

    virtual bool parse_command_line(const int argc, const char * const argv[],
                                    pmdaInterface& interface,
                                    boost::program_options::variables_map &options);

    virtual void initialize_pmda(pmdaInterface &interface);

    virtual pcp::metrics_description get_supported_metrics();

    virtual void begin_fetch_values();

    virtual fetch_value_result fetch_value(const metric_id &metric);

};

#endif
