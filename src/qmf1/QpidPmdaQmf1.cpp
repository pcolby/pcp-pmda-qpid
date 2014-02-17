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

#include "QpidPmdaQmf1.h"

#include <pcp-cpp/atom.hpp>
#include <pcp-cpp/units.hpp>

#include <qpid/log/Logger.h>
#include <qpid/Url.h>

#include "ConsoleUtils.h"

QpidPmdaQmf1::QpidPmdaQmf1() : sessionManager(&consoleListener)
{
    // Setup our instance domain IDs.  Thses instance domains are empty to
    // begin with - we'll dynamically add to them as Qpid updates arrive.
    broker_domain(0);
    queue_domain(1);
    system_domain(2);
}

std::string QpidPmdaQmf1::get_pmda_name() const
{
    return "qpid";
}

int QpidPmdaQmf1::get_default_pmda_domain_number() const
{
    return 124; // Reserved by PCP for Qpid PMDAs.
}

std::string QpidPmdaQmf1::get_pmda_version() const
{
    return "0.0.0";
}

boost::program_options::options_description QpidPmdaQmf1::get_supported_options() const
{
    using namespace boost::program_options;
    options_description connectionOptions("Broker connection options");
    connectionOptions.add_options()
        ("broker,b", value<string_vector>()->
         default_value(string_vector(1, "localhost"), "localhost")
         PCP_CPP_BOOST_PO_VALUE_NAME("url"), "message broker url(s)")
        ("cert-db", value<std::string>()
         PCP_CPP_BOOST_PO_VALUE_NAME("dir"), "path to NSS database")
        ("cert-name", value<std::string>()
         PCP_CPP_BOOST_PO_VALUE_NAME("name"), "name of NSS certificate")
        ("cert-password-file", value<std::string>()
         PCP_CPP_BOOST_PO_VALUE_NAME("file"), "password file for NSS database")
        ("heartbeat", value<double>()
         PCP_CPP_BOOST_PO_VALUE_NAME("interval"), "heartbeat interval in seconds")
        ("locale", value<double>(), "locale to use for Qpid connections")
        ("protocol", value<std::string>(), "version of AMQP to use (e.g. amqp0-10 or amqp1.0)")
        ("tcp-nodelay", bool_switch(), "whether nagle should be enabled")
        ("transport", value<std::string>(), "underlying transport to use (e.g. tcp, ssl, rdma)");
    options_description authenticationOptions("Broker authentication options");
    authenticationOptions.add_options()
        ("username", value<std::string>(), "username to authenticate as")
        ("password", value<std::string>(), "password, if needed by SASL mechanism")
        ("sasl-mechanisms", value<std::string>(), "acceptable SASL mechanisms")
        ("sasl-min-ssf", value<unsigned int>(), "minimum acceptable security strength factor")
        ("sasl-max-ssf", value<unsigned int>(), "maximum acceptable security strength factor")
        ("sasl-service", value<std::string>(), "service name, if needed by SASL mechanism");
    options_description queueOptions("Queue options");
    queueOptions.add_options()
        ("include-auto-delete", bool_switch(), "include auto-delete queues");
    return connectionOptions
            .add(authenticationOptions)
            .add(queueOptions)
            .add(pcp::pmda::get_supported_options());
}

boost::program_options::options_description QpidPmdaQmf1::get_supported_hidden_options() const
{
    using namespace boost::program_options;
    options_description options;
    options.add_options()
        ("no-pmda", bool_switch(), "run as a non-PMDA for development");
    return options;
}

bool QpidPmdaQmf1::parse_command_line(const int argc, const char * const argv[],
                                          pmdaInterface& interface,
                                          boost::program_options::variables_map &options)
{
    // Let the parent implementation do the actually command line parsing.
    if (!pcp::pmda::parse_command_line(argc, argv, interface, options)) {
        return false;
    }

    // Enable Qpid's logging at a level appropriate to the selected debug options.
    if (pmDebug & DBG_TRACE_APPL2) {
        qpid::log::Logger::instance().reconfigure(std::vector<std::string>(1, "trace+"));
    } else if (pmDebug & DBG_TRACE_APPL1) {
        qpid::log::Logger::instance().reconfigure(std::vector<std::string>(1, "debug+"));
    } else {
        qpid::log::Logger::instance().reconfigure(std::vector<std::string>(1, "info+"));
    }

    // Export any NSS options to the environment, as expected by the NSS libraries.
    #define SET_NSS_OPTION(key, name) \
        if (options.count(key)) { \
            const std::string &value = options.at(key).as<std::string>(); \
            if (pmDebug & DBG_TRACE_APPL0) { \
                __pmNotifyErr(LOG_DEBUG, "%s %s=%s", __FUNCTION__, \
                              name, value.c_str()); \
            } \
            if (setenv(name, value.c_str(), true) < 0) { \
                throw pcp::exception(-errno); \
            } \
        }
    SET_NSS_OPTION("cert-db",            "QPID_SSL_CERT_DB")
    SET_NSS_OPTION("cert-name",          "QPID_SSL_CERT_NAME")
    SET_NSS_OPTION("cert-password-file", "QPID_SSL_CERT_PASSWORD_FILE")
    #undef SET_NSS_OPTION

    // Configure our Qpid connection(s) per the command line options.
    const string_vector &brokers = options.at("broker").as<string_vector>();
    for (string_vector::const_iterator iter = brokers.begin(); iter != brokers.end(); ++iter) {
        qpid::client::ConnectionSettings connection;
        #define SET_CONNECTION_OPTION(member, key, type) \
            if (options.count(key)) { \
                connection.member = options.at(key).as<type>(); \
                if (pmDebug & DBG_TRACE_APPL0) { \
                    std::ostringstream stream; \
                    stream << connection.member; \
                    __pmNotifyErr(LOG_DEBUG, "%s %s=%s", __FUNCTION__, \
                                  key, stream.str().c_str()); \
                } \
            }
        SET_CONNECTION_OPTION(virtualhost, "virtualhost", std::string)
        SET_CONNECTION_OPTION(username, "username", std::string)
        SET_CONNECTION_OPTION(password, "password", std::string)
        SET_CONNECTION_OPTION(mechanism, "sasl-mechanism", std::string)
        SET_CONNECTION_OPTION(locale, "locale", std::string)
        SET_CONNECTION_OPTION(heartbeat, "heartbeat", uint16_t)
        SET_CONNECTION_OPTION(maxChannels, "max-channels", uint16_t)
        SET_CONNECTION_OPTION(maxFrameSize, "max-frame-size", uint16_t)
        SET_CONNECTION_OPTION(bounds, "bounds", unsigned int)
        SET_CONNECTION_OPTION(tcpNoDelay, "tcp-no-delay", bool)
        SET_CONNECTION_OPTION(minSsf, "sasl-min-ssf", unsigned int)
        SET_CONNECTION_OPTION(maxSsf, "sasl-min-ssf", unsigned int)
        SET_CONNECTION_OPTION(service, "sasl-service", std::string)
        SET_CONNECTION_OPTION(sslCertName, "ssl-cert-name", std::string)
        SET_CONNECTION_OPTION(protocol, "transport", std::string)
        #undef SET_CONNECTION_OPTION

        const qpid::Url url(*iter);
        __pmNotifyErr(LOG_DEBUG, "%s URL: %s", __FUNCTION__, url.str().c_str());
        if (!url.getUser().empty()) {
            connection.username = url.getUser();
        }
        if (!url.getPass().empty()) {
            connection.password = url.getPass();
        }
        for (qpid::Url::const_iterator i = url.begin(); i != url.end(); ++i) {
            __pmNotifyErr(LOG_DEBUG, "%s Address: %s:%u", __FUNCTION__,
                          i->host.c_str(), i->port);
            connection.host = i->host;
            connection.port = i->port;
            qpidConnectionSettings.push_back(connection);
        }
    }

    consoleListener.setIncludeAutoDelete(
        (options.count("include-auto-delete")) && (options["include-auto-delete"].as<bool>())
    );

    nonPmdaMode = ((options.count("no-pmda") > 0) && (options["no-pmda"].as<bool>()));
    return true;
}

void QpidPmdaQmf1::initialize_pmda(pmdaInterface &interface)
{
    // Setup the QMF console listener.
    for (std::vector<qpid::client::ConnectionSettings>::const_iterator iter = qpidConnectionSettings.begin();
         iter != qpidConnectionSettings.end(); ++iter)
    {
        // Local variable needed because addBroker takes a non-const argument.
        qpid::client::ConnectionSettings connectionSettings(*iter);
        sessionManager.addBroker(connectionSettings);
    }

    // If testing in non-PMDA mode, just wait for input then throw.
    if (nonPmdaMode) {
        std::cout << "Running in non-PMDA mode; outputting to: "
                  << interface.version.two.ext->e_logfile << std::endl
                  << "Press Enter to stop." << std::endl;
        pmdaOpenLog(&interface);
        std::getchar();
        std::cout << "Stopping..." << std::endl;
        #ifdef PM_ERR_FAULT // PM_ERR_FAULT added in PCP 3.6.0.
        throw pcp::exception(PM_ERR_FAULT);
        #else
        throw pcp::exception(PM_ERR_GENERIC);
        #endif
    }

    // Let the parent implementation initialize the rest of the PMDA.
    pcp::pmda::initialize_pmda(interface);
}

pcp::metrics_description QpidPmdaQmf1::get_supported_metrics()
{
    return pcp::metrics_description()
    (0, "broker") // org.apache.qpid.broker::broker::properties
        (0, "connBacklog", pcp::type<uint16_t>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &broker_domain,
         "Connection backlog limit for listening socket")
        (1, "dataDir", pcp::type<std::string>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &broker_domain,
         "Persistent configuration storage location")
        (2, "maxConns", pcp::type<uint16_t>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &broker_domain,
         "Maximum allowed connections")
        (3, "mgmtPubInterval", pcp::type<uint16_t>(), PM_SEM_DISCRETE,
         pcp::units(0,1,0, 0,PM_TIME_SEC,0), &broker_domain,
         "Interval for management broadcasts")
        (4, "mgmtPublish", pcp::type<std::string>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &broker_domain,
         "Broker's management agent sends unsolicited data on the publish interval")
        (5, "name", pcp::type<std::string>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &broker_domain,
         "Index for the broker at this agent")
        (6, "port", pcp::type<uint16_t>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &broker_domain,
         "TCP Port for AMQP Service")
        (7, "stagingThreshold", pcp::type<uint32_t>(), PM_SEM_DISCRETE,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &broker_domain,
         "Broker stages messages over this size to disk")
        (8, "systemRef", pcp::type<std::string>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &broker_domain, "System ID")
        (9, "version", pcp::type<std::string>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &broker_domain,
         "Running software version")
        (10, "workerThreads", pcp::type<uint16_t>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &broker_domain,
         "Thread pool size")
    (1, "broker") // org.apache.qpid.broker::broker::statistics
        (0, "abandoned", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &broker_domain,
         "Messages left in a deleted queue")
        (1, "abandonedViaAlt", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &broker_domain,
         "Messages routed to alternate exchange from a deleted queue")
        (2, "acquires", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &broker_domain,
         "Messages acquired from the queue")
        (3, "byteDepth", pcp::type<uint64_t>(), PM_SEM_INSTANT,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &broker_domain,
         "Current number of bytes on queues in broker")
        (4, "byteFtdDepth", pcp::type<uint64_t>(), PM_SEM_INSTANT,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &broker_domain,
         "Current number of bytes flowed-to-disk")
        (5, "byteFtdDequeues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &broker_domain,
         "Total bytes dequeued from the broker having been flowed-to-disk")
        (6, "byteFtdEnqueues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &broker_domain,
         "Total bytes released from memory and flowed-to-disk on broker")
        (7, "bytePersistDequeues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &broker_domain,
         "Total persistent bytes dequeued from broker")
        (8, "bytePersistEnqueues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &broker_domain,
         "Total persistent bytes enqueued to broker")
        (9, "byteTotalDequeues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &broker_domain,
         "Total bytes dequeued from broker")
        (10, "byteTotalEnqueues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &broker_domain,
         "Total bytes enqueued to broker")
        (11, "byteTxnDequeues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &broker_domain,
         "Total transactional bytes dequeued from broker")
        (12, "byteTxnEnqueues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &broker_domain,
         "Total transactional bytes enqueued to broker")
        (13, "discardsLvq", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &broker_domain,
         "Messages discarded due to LVQ insert")
        (14, "discardsNoRoute", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &broker_domain,
         "Messages discarded due to no-route from exchange")
        (15, "discardsOverflow", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &broker_domain,
         "Messages discarded due to reject-policy overflow")
        (16, "discardsPurge", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &broker_domain,
         "Messages discarded due to management purge")
        (17, "discardsRing", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &broker_domain,
         "Messages discarded due to ring-queue overflow")
        (18, "discardsSubscriber", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &broker_domain,
         "Messages discarded due to subscriber reject")
        (19, "discardsTtl", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &broker_domain,
         "Messages discarded due to TTL expiration")
        (20, "msgDepth", pcp::type<uint64_t>(), PM_SEM_INSTANT,
         pcp::units(0,0,0, 0,0,0), &broker_domain,
         "Current number of messages on queues in broker")
        (21, "msgFtdDepth", pcp::type<uint64_t>(), PM_SEM_INSTANT,
         pcp::units(0,0,0, 0,0,0), &broker_domain,
         "Current number of messages flowed-to-disk")
        (22, "msgFtdDequeues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &broker_domain,
         "Total message bodies dequeued from the broker having been flowed-to-disk")
        (23, "msgFtdEnqueues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &broker_domain,
         "Total message bodies released from memory and flowed-to-disk on broker")
        (24, "msgPersistDequeues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &broker_domain,
         "Total persistent messages dequeued from broker")
        (25, "msgPersistEnqueues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &broker_domain,
         "Total persistent messages enqueued to broker")
        (26, "msgTotalDequeues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &broker_domain,
         "Total messages dequeued from broker")
        (27, "msgTotalEnqueues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &broker_domain,
         "Total messages enqueued to broker")
        (28, "msgTxnDequeues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &broker_domain,
         "Total transactional messages dequeued from broker")
        (29, "msgTxnEnqueues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &broker_domain,
         "Total transactional messages enqueued to broker")
        (30, "queueCount", pcp::type<uint64_t>(), PM_SEM_INSTANT,
         pcp::units(0,0,0, 0,0,0), &broker_domain,
         "Number of queues in the broker")
        (31, "releases", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &broker_domain,
         "Acquired messages reinserted into the queue")
        (32, "reroutes", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &broker_domain,
         "Messages dequeued to management re-route")
        (33, "uptime", pcp::type<uint64_t>(), PM_SEM_INSTANT,
         pcp::units(0,1,0, 0,PM_TIME_NSEC,0), &broker_domain,
         "Total time the broker has been running")
    (2, "queue") // org.apache.qpid.broker::queue::properties
        (0, "altExchange", pcp::type<std::string>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &queue_domain)
        (1, "arguments", pcp::type<std::string>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &queue_domain,
         "Arguments supplied in queue.declare")
        (2, "autoDelete", pcp::type<std::string>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &queue_domain)
        (3, "durable", pcp::type<std::string>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &queue_domain)
        (4, "exclusive", pcp::type<std::string>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &queue_domain)
        (5, "name", pcp::type<std::string>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &queue_domain)
        (6, "vhostRef", pcp::type<std::string>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &queue_domain)
    (3, "queue") // org.apache.qpid.broker::queue::statistics
        (0, "acquires", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Messages acquired from the queue")
        (1, "bindingCountHigh", pcp::type<uint32_t>(), PM_SEM_INSTANT,
         pcp::units(0,0,0, 0,0,0), &queue_domain,
         "Current bindings (High)")
        (2, "bindingCountLow", pcp::type<uint32_t>(), PM_SEM_INSTANT,
         pcp::units(0,0,0, 0,0,0), &queue_domain,
         "Current bindings (Low)")
        (3, "bindingCount", pcp::type<uint32_t>(), PM_SEM_INSTANT,
         pcp::units(0,0,0, 0,0,0), &queue_domain,
         "Current bindings")
        (4, "byteDepth", pcp::type<uint64_t>(), PM_SEM_INSTANT,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &queue_domain,
         "Current size of queue in bytes")
        (5, "byteFtdDepth", pcp::type<uint64_t>(), PM_SEM_INSTANT,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &queue_domain,
         "Current number of bytes flowed-to-disk")
        (6, "byteFtdDequeues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &queue_domain,
         "Total bytes dequeued from the broker having been flowed-to-disk")
        (7, "byteFtdEnqueues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &queue_domain,
         "Total bytes released from memory and flowed-to-disk on broker")
        (8, "bytePersistDequeues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &queue_domain,
         "Persistent messages dequeued")
        (9, "bytePersistEnqueues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &queue_domain,
         "Persistent messages enqueued")
        (10, "byteTotalDequeues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &queue_domain,
         "Total messages dequeued")
        (11, "byteTotalEnqueues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &queue_domain,
         "Total messages enqueued")
        (12, "byteTxnDequeues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &queue_domain,
         "Transactional messages dequeued")
        (13, "byteTxnEnqueues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(1,0,0, PM_SPACE_BYTE,0,0), &queue_domain,
         "Transactional messages enqueued")
        (14, "consumerCountHigh", pcp::type<uint32_t>(), PM_SEM_INSTANT,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Current consumers on queue (High)")
        (15, "consumerCountLow", pcp::type<uint32_t>(), PM_SEM_INSTANT,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Current consumers on queue (Low)")
        (16, "consumerCount", pcp::type<uint32_t>(), PM_SEM_INSTANT,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Current consumers on queue")
        (17, "discardsLvq", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Messages discarded due to LVQ insert")
        (18, "discardsOverflow", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Messages discarded due to reject-policy overflow")
        (19, "discardsPurge", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Messages discarded due to management purge")
        (20, "discardsRing", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Messages discarded due to ring-queue overflow")
        (21, "discardsSubscriber", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Messages discarded due to subscriber reject")
        (22, "discardsTtl", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Messages discarded due to TTL expiration")
        (23, "flowStopped", pcp::type<std::string>(), PM_SEM_INSTANT,
         pcp::units(0,0,0, 0,0,0), &queue_domain, "Flow control active.")
        (24, "flowStoppedCount", pcp::type<uint32_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Number of times flow control was activated for this queue")
        (25, "messageLatencyAverage", pcp::type<uint64_t>(), PM_SEM_INSTANT,
         pcp::units(0,1,0, 0,PM_TIME_NSEC,0), &queue_domain,
         "Broker latency through this queue (Average)")
        (26, "messageLatencyMax", pcp::type<uint64_t>(), PM_SEM_INSTANT,
         pcp::units(0,1,0, 0,PM_TIME_NSEC,0), &queue_domain,
         "Broker latency through this queue (Max)")
        (27, "messageLatencyMin", pcp::type<uint64_t>(), PM_SEM_INSTANT,
         pcp::units(0,1,0, 0,PM_TIME_NSEC,0), &queue_domain,
         "Broker latency through this queue (Min)")
        (28, "messageLatencySamples", pcp::type<uint64_t>(), PM_SEM_INSTANT,
         pcp::units(0,1,0, 0,PM_TIME_NSEC,0), &queue_domain,
         "Broker latency through this queue (Samples)")
        (29, "msgDepth", pcp::type<uint64_t>(), PM_SEM_INSTANT,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Current size of queue in messages")
        (30, "msgFtdDepth", pcp::type<uint64_t>(), PM_SEM_INSTANT,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Current number of messages flowed-to-disk")
        (31, "msgFtdDequeues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Total message bodies dequeued from the broker having been flowed-to-disk")
        (32, "msgFtdEnqueues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Total message bodies released from memory and flowed-to-disk on broker")
        (33, "msgPersistDequeues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Persistent messages dequeued")
        (34, "msgPersistEnqueues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Persistent messages enqueued")
        (35, "msgTotalDequeues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Total messages dequeued")
        (36, "msgTotalEnqueues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Total messages enqueued")
        (37, "msgTxnDequeues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Transactional messages dequeued")
        (38, "msgTxnEnqueues", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Transactional messages enqueued")
        (39, "releases", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Acquired messages reinserted into the queue")
        (40, "reroutes", pcp::type<uint64_t>(), PM_SEM_COUNTER,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Messages dequeued to management re-route")
        (41, "unackedMessagesHigh", pcp::type<uint32_t>(), PM_SEM_INSTANT,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Messages consumed but not yet acked (High)")
        (42, "unackedMessagesLow", pcp::type<uint32_t>(), PM_SEM_INSTANT,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Messages consumed but not yet acked (Low)")
        (43, "unackedMessages", pcp::type<uint32_t>(), PM_SEM_INSTANT,
         pcp::units(0,0,1, 0,0,PM_COUNT_ONE), &queue_domain,
         "Messages consumed but not yet acked")
    (4, "system") // org.apache.qpid.broker::system::properties
        (0, "osName", pcp::type<std::string>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &system_domain, "Operating system name")
        (1, "nodeName", pcp::type<std::string>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &system_domain, "Node name")
        (2, "machine", pcp::type<std::string>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &system_domain)
        (3, "release", pcp::type<std::string>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &system_domain, "System release")
        (4, "version", pcp::type<std::string>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &system_domain, "System version")
        (5, "systemId", pcp::type<std::string>(), PM_SEM_DISCRETE,
         pcp::units(0,0,0, 0,0,0), &system_domain, "System UUID");
}

void QpidPmdaQmf1::begin_fetch_values()
{
    boost::optional<qpid::console::ObjectId> objectId;
    while ((objectId = consoleListener.getNewObjectId())) {
        const boost::optional<qpid::console::Object> props = consoleListener.getProps(*objectId);
        if (!props) {
            __pmNotifyErr(LOG_NOTICE, "No properties found for object %s",
                          ConsoleUtils::toString(*objectId).c_str());
        } else {
            const ConsoleUtils::ObjectSchemaType type = ConsoleUtils::getType(*props);
            pcp::instance_domain * domain = NULL;
            switch (type) {
                case ConsoleUtils::Broker:
                    domain = &broker_domain;
                    break;
                case ConsoleUtils::Queue:
                    domain = &queue_domain;
                    break;
                case ConsoleUtils::System:
                    domain = &system_domain;
                    break;
                default:
                    __pmNotifyErr(LOG_ERR, "%s has unsupported type",
                                  ConsoleUtils::toString(*objectId).c_str());
                    return;
            }
            const std::string instanceName = ConsoleUtils::getName(*props);
            if (instanceName.empty()) {
                __pmNotifyErr(LOG_WARNING, "%s has no name attribute",
                              ConsoleUtils::toString(*objectId).c_str());
                return;
            }
            const int instanceId = pmdaCacheStoreKey(
                *domain, PMDA_CACHE_ADD, instanceName.c_str(),
                 0, NULL, new qpid::console::ObjectId(*objectId));
            if (instanceId < 0) {
                __pmNotifyErr(LOG_ERR, "pmdaCacheStore failed for %s: %s",
                              ConsoleUtils::toString(*objectId).c_str(),
                              pmErrStr(instanceId));
            }
            (*domain)(instanceId, instanceName);
        }
    }
}

pcp::pmda::fetch_value_result QpidPmdaQmf1::fetch_value(const metric_id &metric)
{
    // Get the metric's instance domain.
    pcp::instance_domain * domain = NULL;
    switch (metric.cluster) {
        case 0:
        case 1:
            domain = &broker_domain;
            break;
        case 2:
        case 3:
            domain = &queue_domain;
            break;
        case 4:
            domain = &system_domain;
            break;
    }

    // Fetch the Qpid objectId from the PMDA cache (we added in begin_fetch_values).
    char * instanceName = NULL;
    void * opaque;
    const int status = pmdaCacheLookup(*domain, metric.instance, &instanceName, &opaque);
    if ((status != PMDA_CACHE_ACTIVE) && (status != PMDA_CACHE_INACTIVE)) {
        __pmNotifyErr(LOG_NOTICE, "pmdaCacheLookup failed for cluster %ju: %s",
                      (uintmax_t)metric.cluster, pmErrStr(status));
        throw pcp::exception(PM_ERR_INST);
    }
    if (opaque == NULL) {
        __pmNotifyErr(LOG_ERR, "pmdaCacheLookup returned NULL for cluster %ju",
                      (uintmax_t)metric.cluster);
        throw pcp::exception(PM_ERR_INST);
    }
    const qpid::console::ObjectId * const objectId = static_cast<qpid::console::ObjectId *>(opaque);

    // Fetch the object's propeties or statistics, according to the metric cluster.
    const boost::optional<qpid::console::Object> object = (metric.cluster % 2 == 0)
        ? consoleListener.getProps(*objectId) : consoleListener.getStats(*objectId);
    if (!object) {
        __pmNotifyErr(LOG_NOTICE, "no %s for %s",
                      (metric.cluster % 2 == 0) ? "properties" : "statistics",
                      ConsoleUtils::toString(*object).c_str());
        throw pcp::exception(PM_ERR_INST);
    }

    // Get the name of metric corresponding to the metric item, and fetch the metric.
    const std::string &metricName = supported_metrics.at(metric.cluster).at(metric.item).metric_name;
    const qpid::console::Object::AttributeMap &attributes = object->getAttributes();
    const qpid::console::Object::AttributeMap::const_iterator attribute = attributes.find(metricName);
    if (attribute == attributes.end()) {
        __pmNotifyErr(LOG_NOTICE, "no %s metric found for %s", metricName.c_str(),
                      ConsoleUtils::toString(*object).c_str());
        throw pcp::exception(PM_ERR_VALUE);
    }

    // Return the metric value as a PCP atom.
    try{
        switch (metric.type) {
            case PM_TYPE_32:     return pcp::atom(metric.type, attribute->second->asInt());
            case PM_TYPE_64:     return pcp::atom(metric.type, attribute->second->asInt64());
            case PM_TYPE_U32:    return pcp::atom(metric.type, attribute->second->asUint());
            case PM_TYPE_U64:    return pcp::atom(metric.type, attribute->second->asUint64());
            case PM_TYPE_FLOAT:  return pcp::atom(metric.type, attribute->second->asFloat());
            case PM_TYPE_DOUBLE: return pcp::atom(metric.type, attribute->second->asDouble());
            case PM_TYPE_STRING:
                if (attribute->second->isBool()) {
                    return pcp::atom(metric.type, strdup(
                        attribute->second->asBool() ? "true" : "false"));
                } else if (attribute->second->isMap()) {
                    std::ostringstream stream;
                    stream << attribute->second->asMap();
                    return pcp::atom(metric.type, strdup(stream.str().c_str()));
                } else if (attribute->second->isNull()) {
                    return pcp::atom(metric.type, strdup("null"));
                } else if (attribute->second->isObjectId()) {
                    return pcp::atom(metric.type, strdup(
                        ConsoleUtils::toString(attribute->second->asObjectId()).c_str()));
                } else if (attribute->second->isUuid()) {
                    return pcp::atom(metric.type, strdup(
                        attribute->second->asUuid().str().c_str()));
                } else {
                    return pcp::atom(metric.type, strdup(
                        attribute->second->asString().c_str()));
                }
            default:
                __pmNotifyErr(LOG_ERR, "%s metric uses unsupported type %d",
                              metricName.c_str(), metric.type);
                throw pcp::exception(PM_ERR_TYPE);
        }
    } catch (const qpid::Exception &ex) {
        __pmNotifyErr(LOG_ERR, "error converting %s metric to type %d: %s",
                      metricName.c_str(), metric.type, ex.what());
        throw pcp::exception(PM_ERR_TYPE, ex.getMessage());
    }
}
