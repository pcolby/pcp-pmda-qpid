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
    bool nonPmdaMode;
    std::vector<qpid::client::ConnectionSettings> qpidConnectionSettings;

    pcp::instance_domain broker_domain, queue_domain, system_domain;

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

        nonPmdaMode = ((options.count("no-pmda") > 0) && (options["no-pmda"].as<bool>()));
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
        if (nonPmdaMode) {
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
            (4, "mgmtPublish", pcp::type<uint8_t>(), PM_SEM_DISCRETE,
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
            (2, "autoDelete", pcp::type<uint8_t>(), PM_SEM_DISCRETE,
             pcp::units(0,0,0, 0,0,0), &queue_domain)
            (3, "durable", pcp::type<uint8_t>(), PM_SEM_DISCRETE,
             pcp::units(0,0,0, 0,0,0), &queue_domain)
            (4, "exclusive", pcp::type<uint8_t>(), PM_SEM_DISCRETE,
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
            (23, "flowStopped", pcp::type<uint8_t>(), PM_SEM_INSTANT,
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

    virtual fetch_value_result fetch_value(const metric_id &metric)
    {
        throw pcp::exception(PM_ERR_NYI);
        return pcp::atom(metric.type,time(NULL));
    }

};

int main(int argc, char *argv[])
{
    return pcp::pmda::run_daemon<QpidPmda>(argc, argv);
}
