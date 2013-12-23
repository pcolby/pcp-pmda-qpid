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

};

int main(int argc, char *argv[])
{
    return pcp::pmda::run_daemon<QpidPmda>(argc, argv);
}
