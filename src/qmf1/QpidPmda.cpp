/// Copyright Paul Colby 2013.
/// @todo Add (Apache?) license.

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
        options_description options("Options");
        options.add_options()
            ("broker,b", value<std::string>()->default_value("ampq://localhost:5672")
             PCP_CPP_BOOST_PO_VALUE_NAME("url"), "broker url");
        options.add(pcp::pmda::supported_options());
        return options;
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
