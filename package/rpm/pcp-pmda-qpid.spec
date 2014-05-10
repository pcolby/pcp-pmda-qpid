#
# RPM Spec file for the PMDA++ project.
#

%{!?pcp_pmdas_dir: %global pcp_pmdas_dir %(pmconfig PCP_PMDAS_DIR | cut -c15- )}

Summary: PCP PMDA for Qpid
Name: pcp-pmda-qpid
Version: 0.2.3
Release: 1%{?dist}
License: Apache License
Group: Development/Libraries
Source: pcp-pmda-qpid-%{version}.tar.gz
URL: https://github.com/pcolby/pcp-pmda-qpid

Requires: boost-program-options
Requires: pcp-libs
Requires: qpid-cpp-client
Requires: qpid-qmf

BuildRequires: boost >= 1.32
BuildRequires: cmake >= 2.6
BuildRequires: pcp-libs-devel
BuildRequires: pcp-pmda-cpp-devel >= 0.3.4
BuildRequires: qpid-cpp-client-devel
BuildRequires: qpid-qmf-devel
BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root-%(%{__id_u} -n)

%description
Qpid PMDA is a free, open source, PCP Performance Metrics Domain Agent for
Apache Qpid. It exposes performance metrics from Qpid message brokers via QMF,
to Performance Co-Pilot (PCP) using the PMDA++ library.

%prep
%setup -c -q

%build
%{__rm} -rf %{buildroot}
%cmake %{name}-%{version}
%{__make} %{?_smp_mflags}

%install
%{__make} DESTDIR=%{buildroot} install

%clean
%{__rm} -rf %{buildroot}

%files
%{pcp_pmdas_dir}/qpid/domain.h
%{pcp_pmdas_dir}/qpid/help
%{pcp_pmdas_dir}/qpid/help.dir
%{pcp_pmdas_dir}/qpid/help.pag
%{pcp_pmdas_dir}/qpid/Install
%{pcp_pmdas_dir}/qpid/pmdaqpid
%{pcp_pmdas_dir}/qpid/pmns
%{pcp_pmdas_dir}/qpid/Remove
%{pcp_pmdas_dir}/qpid/root

%changelog
* Sat May 10 2014 Paul Colby <git@colby.id.au> - 0.2.3-1
- updated to %{name} 0.2.3.
- added explicit files list.

* Fri Mar 21 2014 Paul Colby <git@colby.id.au> - 0.2.2-1
- updated to %{name} 0.2.2.

* Wed Mar 05 2014 Paul Colby <git@colby.id.au> - 0.2.1-1
- updated to %{name} 0.2.1.

* Tue Feb 18 2014 Paul Colby <git@colby.id.au> - 0.2.0-1
- updated to %{name} 0.2.0.

* Sun Feb 16 2014 Paul Colby <git@colby.id.au> - 0.1.1-1
- updated to %{name} 0.1.1.

* Sat Feb 15 2014 Paul Colby <git@colby.id.au> - 0.1.0-1
- initial %{name} spec file.
