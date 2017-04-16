# Qpid PMDA
[![Build Status](http://img.shields.io/travis/pcolby/pcp-pmda-qpid.svg)](https://travis-ci.org/pcolby/pcp-pmda-qpid)
[![Github Release](http://img.shields.io/github/release/pcolby/pcp-pmda-qpid.svg)](https://github.com/pcolby/pcp-pmda-qpid/releases/latest)
[![Apache License](http://img.shields.io/badge/license-APACHE2-blue.svg)](https://www.apache.org/licenses/LICENSE-2.0.html)


Qpid PMDA is free and open source, [PCP](http://oss.sgi.com/projects/pcp/
"Performance Co-Pilot") Performance Metrics Domain Agent for [Apache Qpid](
http://qpid.apache.org/). It exposes performance metrics from Qpid message
brokers via [QMF](http://qpid.apache.org/components/qmf/index.html "Qpid
Management Framework"), to Performance Co-Pilot (PCP) using the [PMDA++](
https://github.com/pcolby/pcp-pmda-cpp) library.

```
      PCP      |   libpcp    |        | pmdaqpid-qmf1 | QMF1 |     Qpid
  Monitoring   |-------------| PMDA++ |---------------+------|   Messaging
Infrastructure | libpcp_pmda |        | pmdaqpid-qmf2 | QMF2 | Infrastructure
```

## Requirements

* [Boost C++ Libraries](http://www.boost.org/) 1.32+
* [PMDA++](https://github.com/pcolby/pcp-pmda-cpp) 0.4.0+
* [Apache Qpid](http://qpid.apache.org/)

## Building & Installing
1. Install the pre-requisites - Boost, Apache Qpid, [PMDA++](https://github.com/pcolby/pcp-pmda-cpp).
2. `mkdir build && cd build`
3. `cmake <path-to-code> && make && sudo make install`
4. ``cd `pmconfig PCP_PMDAS_DIR | cut -b15-`/qpid && ./Install``

Alternatively, use [rpmbuild](package/rpm).

## Contact

Issues / suggestions can be reported via GitHub's [issue tracker](
https://github.com/pcolby/pcp-pmda-qpid/issues) (pull requests are very
welcome).

The [PMDA++ Google Group](http://groups.google.com/group/pcp-pmda-cpp/) is also
used for general discussion, questions, comments, suggestions, etc for this
project.  Email pcp-pmda-cpp+subscribe@googlegroups.com to subscribe.

## License

Available under the [Apache License, Version 2.0](
http://apache.org/licenses/LICENSE-2.0.html).
