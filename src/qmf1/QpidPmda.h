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

#ifndef __QPID_PMDA_QPID_PMDA_H__
#define __QPID_PMDA_QPID_PMDA_H__

#include <pcp-cpp/instance_domain.hpp>
#include <pcp-cpp/pmda.hpp>

#include <qpid/client/ConnectionSettings.h>
#include <qpid/console/SessionManager.h>

#include "ConsoleListener.h"
#include "ConsoleUtils.h"

class QpidPmda : public pcp::pmda {

public:
    QpidPmda();

    virtual std::string get_pmda_name() const;

    virtual int get_default_pmda_domain_number() const;

    virtual std::string get_pmda_version() const;

protected:
    bool nonPmdaMode;
    std::vector<qpid::client::ConnectionSettings> qpidConnectionSettings;

    pcp::instance_domain broker_domain, queue_domain, system_domain;

    ConsoleListener consoleListener;
    qpid::console::SessionManager sessionManager;

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
