%define lib_path lmicc

Name:           lmicc
Version:        0.1.0
Release:        2%{?dist}
Summary:        GUI for OpenLMI
License:        GPLv2.1+, LGPLv2.1+
URL:            https://github.com/mhatina/openlmi_gui 
Source0:        lmicc.tar.gz 

BuildRequires:  qt-devel >= 4.8.5
BuildRequires:  tog-pegasus-devel >= 2.12.0
BuildRequires:  tog-pegasus-libs >= 2.12.0
BuildRequires:  libgnome-keyring-devel >= 3.10.1
BuildRequires:  boost-devel >= 1.54.0
BuildRequires:  openslp-devel >= 1.2.1
BuildRequires:  lmicc-logger
Requires:       tog-pegasus >= 2.12.0
Requires:       boost >= 1.54.0
Requires:       boost-thread >= 1.54.0
Requires:       openslp >= 1.2.1
Requires:       libgnome-keyring >= 3.10.1
Requires:       lmicc-bin

%package bin
Summary:        binary for %{name}
Group:          System/Management
Requires:       lmicc-logger
Requires:       lmicc-overview

%package doc
Summary:        Documentation for %{name}
Group:          Documentation

%package libs
Summary:        Collection of libs for GUI for OpenLMI
Group:          Development/Libraries
Requires:       lmicc-account
Requires:       lmicc-hardware
Requires:       lmicc-network
Requires:       lmicc-service
Requires:       lmicc-software

%package account
Summary:        Account tab for %{summary}
Group:          Development/Libraries

%package hardware
Summary:        Hardware tab for %{summary}
Group:          Development/Libraries

%package network
Summary:        Network tab for %{summary}
Group:          Development/Libraries

%package overview
Summary:        Overview tab for %{summary}
Group:          Development/Libraries

%package service
Summary:        Service tab for %{summary}
Group:          Development/Libraries

%package software
Summary:        Software tab for %{summary}
Group:          Development/Libraries

%description
%{name} is a Graphical User Interface for OpenLMI.

%description bin
%{summary}

%description doc
%{summary}

%description libs
%{summary}

%description account
%{summary} 

%description hardware
%{summary}      

%description network
%{summary} 

%description overview
%{summary} 

%description service
%{summary}      

%description software
%{summary} 

%prep
%setup -q -n %{name}-%{version}

%build
qmake-qt4 PREFIX=%{buildroot}/usr/ LIB_PATH=%{_lib}
make 

%install
make -C src/ui install
make -C src/plugin install
export QA_SKIP_BUILD_ROOT="true"

%files

%files bin
%defattr(-,root,root,0755)
%doc LICENSE README.md
%{_bindir}/lmicc 

#%files doc
#%dir %{_docdir}/%{name}

%files libs

%files account
%{_libdir}/%{lib_path}/liblmiccaccount*

%files hardware
%{_libdir}/%{lib_path}/liblmicchardware*

%files network
%{_libdir}/%{lib_path}/liblmiccnetwork*

%files overview
%{_libdir}/%{lib_path}/liblmiccoverview*

%files service
%{_libdir}/%{lib_path}/liblmiccservice*

%files software
%{_libdir}/%{lib_path}/liblmiccsoftware*

%changelog
* Fri Sep 19 2014 Martin Hatina <mhatina@redhat.com> - 0.1.0-2
- bug fixes

* Thu Sep 18 2014 Martin Hatina <mhatina@redhat.com> - 0.1.0-1
- lot of new features and bug fixes

* Tue Aug 26 2014 Martin Hatina <mhatina@redhat.com> - 0.0.1-3
- plugin path related bug fix

* Mon Aug 25 2014 Martin Hatina <mhatina@redhat.com> - 0.0.1-2
- bug fixes, added network tab

