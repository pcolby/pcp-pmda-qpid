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

#include <pcp-cpp/atom.hpp>
#include <pcp-cpp/pmda.hpp>
#include <pcp-cpp/units.hpp>

#include "ConsoleListener.hpp"

class QpidPmda : public pcp::pmda {

public:

    virtual std::string get_pmda_name() const
    {
        return "qpid";
    }

    virtual int default_pmda_domain_number() const
    {
        return 123; /// @todo Pick something appropriate.
    }

protected:

    virtual boost::program_options::options_description supported_options() const
    {
        using namespace boost::program_options;
        options_description connectionOptions("Broker connection options");
        connectionOptions.add_options()
            ("broker,b", value<string_vector>()->
             default_value(string_vector(1, "ampq://localhost:5672"), "ampq://localhost:5672")
             PCP_CPP_BOOST_PO_VALUE_NAME("url"), "message broker url(s)")
            ("heartbeat", value<double>()
             PCP_CPP_BOOST_PO_VALUE_NAME("interval"), "heartbeat interval in seconds")
            ("protocol", value<std::string>(), "version of AMQP to use (e.g. amqp0-10 or amqp1.0)")
            ("tcp-nodelay", value<bool>()PCP_CPP_BOOST_PO_IMPLICIT_VALUE(true),
             "whether nagle should be enabled")
            ("transport", value<std::string>(), "underlying transport to use (e.g. tcp, ssl, rdma)");
        options_description authenticationOptions("Broker authentication options");
        authenticationOptions.add_options()
            ("username", value<std::string>(), "username to authenticate as")
            ("password", value<std::string>(), "password, if needed by SASL mechanism")
            ("sasl-mechanisms", value<std::string>(), "acceptable SASL mechanisms")
            ("sasl-min-ssf", value<unsigned>(), "minimum acceptable security strength factor")
            ("sasl-max-ssf", value<unsigned>(), "maximum acceptable security strength factor")
            ("sasl-service", value<std::string>(), "service name, if needed by SASL mechanism");
        return connectionOptions.add(authenticationOptions).add(pcp::pmda::supported_options());
    }

    virtual pcp::metrics_description get_supported_metrics()
    {
        /// @todo  Lots and lots of metrics! :)
        return pcp::metrics_description();
    }

    virtual fetch_value_result fetch_value(const metric_id &metric)
    {
        throw pcp::exception(PM_ERR_NYI);
        return pcp::atom(metric.type,time(NULL));
    }

    /// @todo  Override somewhere to register our console listener.

};

int main(int argc, char *argv[])
{
    return pcp::pmda::run_daemon<QpidPmda>(argc, argv);
}
