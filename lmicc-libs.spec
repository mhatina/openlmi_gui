Name:           lmicc-libs
Version:        0.0.1
Release:        1%{?dist}
Summary:        GUI for OpenLMI
License:        GPLv2.1+, LGPLv2.1+
URL:            https://github.com/mhatina/openlmi_gui 
Source0:        lmicc.tar.gz 

BuildRequires:  qt-devel >= 4.8.5
BuildRequires:  tog-pegasus-devel >= 2.12.0
BuildRequires:  tog-pegasus-libs >= 2.12.0
BuildRequires:  boost-devel >= 1.54.0
Requires:       tog-pegasus >= 2.12.0
Requires:       boost >= 1.54.0
Requires:       boost-thread >= 1.54.0

%package doc
Summary:        Documentation for %{name}
Group:          Documentation

%package -n lmicc-account
Summary:        Account provider for %{summary}
Group:          Development/Libraries

%package -n lmicc-hardware
Summary:        Hardware provider for %{summary}
Group:          Development/Libraries

%package -n lmicc-network
Summary:        Network provider for %{summary}
Group:          Development/Libraries

%package -n lmicc-service
Summary:        Service provider for %{summary}
Group:          Development/Libraries

%package -n lmicc-software
Summary:        Software provider for %{summary}
Group:          Development/Libraries

%description
%{name} is collection of libraries for Graphical User Interface for OpenLMI.

%description doc
%{summary}

%description -n lmicc-account
%{summary}

%description -n lmicc-hardware
%{summary}

%description -n lmicc-network
%{summary}

%description -n lmicc-service
%{summary}

%description -n lmicc-software
%{summary}

%prep
%setup -q -n lmicc-%{version}

%define library_path lmicc

%build
qmake-qt4 target.path=%{buildroot}%{_libdir}/%{library_path} 
make 

%install
make -C src/plugin install

%files
%defattr(-,root,root,0755)  
%doc LICENSE README.md
%{_libdir}/%{library_path}/lib*

%files doc
%dir %{_docdir}/%{name}

%files -n lmicc-account
%{_libdir}/%{library_path}/liblmiccaccount*

%files -n lmicc-hardware
%{_libdir}/%{library_path}/liblmicchardware*

%files -n lmicc-network
%{_libdir}/%{library_path}/liblmiccnetwork*

%files -n lmicc-service
%{_libdir}/%{library_path}/liblmiccservice*

%files -n lmicc-software
%{_libdir}/%{library_path}/liblmiccsoftware*

%changelog
