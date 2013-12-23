/// Copyright Paul Colby 2013.
/// @todo Add (Apache?) license.

#include <qpid/console/ConsoleListener.h>

class MyConsoleListener : public qpid::console::ConsoleListener {

public:
    virtual void brokerConnected(const qpid::console::Broker &broker);

    virtual void brokerDisconnected(const qpid::console::Broker &broker);

    virtual void newPackage(const std::string &package);

    virtual void newClass(const qpid::console::ClassKey &classKey);

    virtual void newAgent(const qpid::console::Agent &agent);

    virtual void delAgent (const qpid::console::Agent &agent);

    virtual void objectProps(qpid::console::Broker &broker, qpid::console::Object &object);

    virtual void objectStats(qpid::console::Broker &broker, qpid::console::Object &object);

    virtual void event(qpid::console::Event &event);

    virtual void brokerInfo(qpid::console::Broker &broker);

};
