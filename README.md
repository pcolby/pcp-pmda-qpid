# Qpid PMDA [![Build Status](https://travis-ci.org/pcolby/pcp-pmda-qpid.png?branch=master)](https://travis-ci.org/pcolby/pcp-pmda-qpid)

Qpid PMDA is a free, open source, [PCP](http://oss.sgi.com/projects/pcp/
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
* [PMDA++](https://github.com/pcolby/pcp-pmda-cpp) 0.3.3+
* [Apache Qpid](http://qpid.apache.org/)

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
