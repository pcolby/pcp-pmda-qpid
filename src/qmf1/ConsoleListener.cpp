/// Copyright Paul Colby 2013.
/// @todo Add (Apache?) license.

#include "ConsoleListener.hpp"

#include <pcp/pmapi.h>
#include <pcp/impl.h>

void ConsoleListener::brokerConnected(const qpid::console::Broker &broker) {
    __pmNotifyErr(LOG_DEBUG, "broker (%s) %s connected",
                  broker.getUrl().c_str(), broker.getBrokerId().str().c_str());
}

void ConsoleListener::brokerDisconnected(const qpid::console::Broker &broker) {
    //std::cout << "broker: " << broker.getUrl() << " disconnected" << std::endl;
}

void ConsoleListener::newPackage(const std::string &package) {
    //std::cout << "package: " << package << std::endl;
}

void ConsoleListener::newClass(const qpid::console::ClassKey &classKey) {
    //std::cout << "class: " << classKey.getClassName() << std::endl;
}

void ConsoleListener::newAgent(const qpid::console::Agent &agent) {
    //std::cout << "newAgent: " << agent.getLabel() << std::endl;
}

void ConsoleListener::delAgent (const qpid::console::Agent &agent) {
    //std::cout << "delAgent: " << agent.getLabel() << std::endl;
}

void ConsoleListener::objectProps(qpid::console::Broker &broker, qpid::console::Object &object) {
    //std::cout << "object: " << object.getObjectId();
    //const Object::AttributeMap::const_iterator name = object.getAttributes().find("name");
    //if (name != object.getAttributes().end())
        //std::cout << ' ' << name->second->str();
    //std::cout << std::endl;

    //for (Object::AttributeMap::const_iterator attribute = object.getAttributes().begin();
        //attribute != object.getAttributes().end(); attribute++) {
        //std::cout << "  attribute: " << attribute->first << " => "
          //        << attribute->second->str() << std::endl;
    //}
}

void ConsoleListener::objectStats(qpid::console::Broker &broker, qpid::console::Object &object) {
}

void ConsoleListener::event(qpid::console::Event &event) {
    //std::cout << "event: " << event.getClassKey().getClassName() << std::endl;
    //for (Object::AttributeMap::const_iterator attribute = event.getAttributes().begin();
        //attribute != event.getAttributes().end(); attribute++) {
        //std::cout << "  attribute: " << attribute->first << " => "
          //        << attribute->second->str() << std::endl;
    //}
}

void ConsoleListener::brokerInfo(qpid::console::Broker &broker) {
    //std::cout << "brokerInfo: " << broker.getUrl() << std::endl;
}
