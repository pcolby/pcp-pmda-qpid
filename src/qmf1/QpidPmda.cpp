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

#include <qpid/console/SessionManager.h>
#include <qpid/Url.h>

#include <boost/regex.hpp>

#include "ConsoleListener.hpp"

class QpidPmda : public pcp::pmda {

public:

    virtual std::string get_pmda_name() const
    {
        return "qpid";
    }

    virtual int get_default_pmda_domain_number() const
    {
        return 123; /// @todo Pick something appropriate.
    }

protected:
    std::vector<qpid::client::ConnectionSettings> qpidConnectionSettings;

    virtual boost::program_options::options_description get_supported_options() const
    {
        using namespace boost::program_options;
        options_description connectionOptions("Broker connection options");
        connectionOptions.add_options()
            ("broker,b", value<string_vector>()->
             default_value(string_vector(1, "localhost"), "localhost")
             PCP_CPP_BOOST_PO_VALUE_NAME("url"), "message broker url(s)")
            ("heartbeat", value<double>()
             PCP_CPP_BOOST_PO_VALUE_NAME("interval"), "heartbeat interval in seconds")
            ("locale", value<double>(), "locale to use for Qpid connections")
            ("protocol", value<std::string>(), "version of AMQP to use (e.g. amqp0-10 or amqp1.0)")
            ("tcp-nodelay", value<bool>()PCP_CPP_BOOST_PO_IMPLICIT_VALUE(false),
             "whether nagle should be enabled")
            ("transport", value<std::string>(), "underlying transport to use (e.g. tcp, ssl, rdma)");
        options_description authenticationOptions("Broker authentication options");
        authenticationOptions.add_options()
            ("username", value<std::string>(), "username to authenticate as")
            ("password", value<std::string>(), "password, if needed by SASL mechanism")
            ("sasl-mechanisms", value<std::string>(), "acceptable SASL mechanisms")
            ("sasl-min-ssf", value<unsigned int>(), "minimum acceptable security strength factor")
            ("sasl-max-ssf", value<unsigned int>(), "maximum acceptable security strength factor")
            ("sasl-service", value<std::string>(), "service name, if needed by SASL mechanism");
        return connectionOptions.add(authenticationOptions).add(pcp::pmda::get_supported_options());
    }

    virtual boost::program_options::options_description get_supported_hidden_options() const
    {
        using namespace boost::program_options;
        options_description options;
        options.add_options()
            ("no-pmda", bool_switch(), "run as a non-PMDA for development");
        return options;
    }

    virtual bool parse_command_line(const int argc, const char * const argv[],
                                    pmdaInterface& interface,
                                    boost::program_options::variables_map &options)
    {
        // Let the parent implementation do the actually command line parsing.
        if (!pcp::pmda::parse_command_line(argc, argv, interface, options)) {
            return false;
        }

        // Configure our Qpid connection(s) per the command line options.
        const string_vector &brokers = options.at("broker").as<string_vector>();
        for (string_vector::const_iterator iter = brokers.begin(); iter != brokers.end(); ++iter) {
            qpid::client::ConnectionSettings connection;
            #define SET_CONNECTION_OPTION(member, key, type) \
                if (options.count(key)) \
                    connection.member = options.at(key).as<type>()
            SET_CONNECTION_OPTION(virtualhost, "virtualhost", std::string);
            SET_CONNECTION_OPTION(username, "username", std::string);
            SET_CONNECTION_OPTION(password, "password", std::string);
            SET_CONNECTION_OPTION(mechanism, "sasl-mechanism", std::string);
            SET_CONNECTION_OPTION(locale, "locale", std::string);
            SET_CONNECTION_OPTION(heartbeat, "heartbeat", uint16_t);
            SET_CONNECTION_OPTION(maxChannels, "max-channels", uint16_t);
            SET_CONNECTION_OPTION(maxFrameSize, "max-frame-size", uint16_t);
            SET_CONNECTION_OPTION(bounds, "bounds", unsigned int);
            SET_CONNECTION_OPTION(tcpNoDelay, "tcp-no-delay", bool);
            SET_CONNECTION_OPTION(minSsf, "sasl-min-ssf", unsigned int);
            SET_CONNECTION_OPTION(maxSsf, "sasl-min-ssf", unsigned int);
            SET_CONNECTION_OPTION(service, "sasl-service", std::string);
            SET_CONNECTION_OPTION(sslCertName, "ssl-cert-name", std::string);
            #undef SET_CONNECTION_OPTION

            qpid::Url url(*iter);
            __pmNotifyErr(LOG_DEBUG, "%s:%d:%s %s", __FILE__, __LINE__, __FUNCTION__, url.str().c_str());
            if (!url.getUser().empty()) {
                connection.username = url.getUser();
            }
            if (!url.getPass().empty()) {
                connection.password = url.getPass();
            }
            for (qpid::Url::const_iterator i = url.begin(); i != url.end(); ++i) {
                __pmNotifyErr(LOG_DEBUG, "%s:%d:%s %s:%s:%u", __FILE__, __LINE__, __FUNCTION__,
                              i->protocol.c_str(), i->host.c_str(), i->port);
                connection.protocol = i->protocol;
                connection.host = i->host;
                connection.port = i->port;
                qpidConnectionSettings.push_back(connection);
            }
        }
        return true;
    }

    virtual void initialize_pmda(pmdaInterface &interface)
    {
        // Setup the QMF console listener.
        ConsoleListener consoleListener; // Add param for debug / log mode?
        qpid::console::SessionManager sessionManager(&consoleListener);
        for (std::vector<qpid::client::ConnectionSettings>::const_iterator iter = qpidConnectionSettings.begin();
             iter != qpidConnectionSettings.end(); ++iter)
        {
            // Local variable needed because addBroker takes a non-const argument.
            qpid::client::ConnectionSettings connectionSettings(*iter);
            sessionManager.addBroker(connectionSettings);
        }

        // If testing in non-PMDA mode, just wait for input then throw.
        if (true) { /// @todo This comes from the CLI option.
            std::cout << "Running in non-PMDA mode; outputting to: "
                      << interface.version.two.ext->e_logfile << std::endl
                      << "Press Enter to stop." << std::endl;
            pmdaOpenLog(&interface);
            std::getchar();
            std::cout << "Stopping..." << std::endl;
            throw pcp::exception(PM_ERR_FAULT);
        }

        // Let the parent implementation initialize the rest of the PMDA.
        pcp::pmda::initialize_pmda(interface);
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
