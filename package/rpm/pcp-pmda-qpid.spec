#
# RPM Spec file for the PMDA++ project.
#

%{!?pcp_pmdas_dir: %global pcp_pmdas_dir %(pmconfig PCP_PMDAS_DIR | cut -c15- )}

Summary: PCP PMDA for Qpid
Name: pcp-pmda-qpid
Version: 0.1.0
Release: 1%{?dist}
License: Apache License
Group: Development/Libraries
Source: pcp-pmda-qpid-%{version}.tar.gz
URL: https://github.com/pcolby/pcp-pmda-qpid

BuildRequires: boost >= 1.32
BuildRequires: cmake >= 2.6
BuildRequires: pcp-libs-devel
BuildRequires: pcp-pmda-cpp-devel
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

%package qmf1
Summary: Qpid PMDA using QMFv1
Group: Development/Libraries
Requires: boost-progrma-options
Requires: pcp-libs
Requires: qpid-cpp-client
Requires: qpid-qmf

%description qmf1
Qpid PMDA using QMFv1.

%files qmf1
%{pcp_pmdas_dir}/qpid-qmf1

%changelog
* Sat Feb 15 2014 Paul Colby <git@colby.id.au> - 0.1.0-1
- initial %{name} spec file.
