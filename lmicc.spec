Name:           lmicc
Version:        0.0.1
Release:        1%{?dist}
Summary:        GUI for OpenLMI
License:        GPLv2.1+, LGPLv2.1+
URL:            https://github.com/mhatina/openlmi_gui 
Source0:        lmicc.tar.gz 
source1:        icons.tar.gz

BuildRequires:  qt-devel >= 4.8.5
BuildRequires:  tog-pegasus-devel >= 2.12.0
BuildRequires:  tog-pegasus-libs >= 2.12.0
BuildRequires:  libgnome-keyring-devel >= 3.10.1
BuildRequires:  boost-devel >= 1.54.0
BuildRequires:  openslp-devel >= 1.2.1 
Requires:       tog-pegasus >= 2.12.0
Requires:       boost >= 1.54.0
Requires:       boost-thread >= 1.54.0
Requires:       openslp >= 1.2.1

%package doc
Summary:        Documentation for %{name}
Group:          Documentation

%package account
Summary:        Account provider for %{name}
Group:          Provider

%package hardware
Summary:        Hardware provider for %{name}
Group:          Provider

%package network
Summary:        Network provider for %{name}
Group:          Provider

%package service
Summary:        Service provider for %{name}
Group:          Provider

%package software
Summary:        Software provider for %{name}
Group:          Provider

%description
%{name} is a Graphical User Interface for OpenLMI.

%description doc
%{summary}

%description account
%{summary}

%description hardware
%{summary}

%description network
%{summary}

%description service
%{summary}

%description software
%{summary}

%prep
%setup -q -n %{name}-%{version}

%build
qmake-qt4
make -C logger
make -C logger install
make 

%install
make DESTDIR=%{buildroot} install
find %{buildroot} -name '*.la' | xargs rm -f

%files
%defattr(-,root,root,0755)  
%doc COPYING README.md
%{_libdir}/liblmicclogger.so.*
%{_bindir}/lmicc 

%files doc
%dir %{_docdir}/%{name}-%{version}
%{_docdir}/%{name}-%{version}/html

%files account
%{_libdir}/%{name}/libaccountProvider.so

%files hardware
%{_libdir}/%{name}/libhardwareProvider.so

%files network
%{_libdir}/%{name}/libnetworkProvider.so

%files service
%{_libdir}/%{name}/libserviceProvider.so

%files software
%{_libdir}/%{name}/libsoftwareProvider.so

%changelog
