Name:           lmicc-logger
Version:        0.0.1
Release:        4%{?dist}
Summary:        Logger for lmicc
License:        GPLv2.1+, LGPLv2.1+
URL:            https://github.com/mhatina/openlmi_gui 
Source0:        lmicc.tar.gz 

BuildRequires:  qt-devel >= 4.8.5

%package doc
Summary:        Documentation for %{name}
Group:          Documentation

%description
%{name} is a logger for Graphical User Interface for OpenLMI.

%description doc
%{summary}

%prep
%setup -q -n lmicc-%{version}

%build
cd src/logger
qmake-qt4 PREFIX=%{buildroot} LIB_PATH=%{_libdir}
make  
cd -

%install
make -C src/logger install

%files
%defattr(-,root,root,0755)  
%doc LICENSE README.md
%{_libdir}/lib*

%files doc
%dir %{_docdir}/%{name}

%changelog
* Mon Aug 25 2014 Martin Hatina <mhatina@redhat.com> - 0.0.1-4
-

* Mon Aug 25 2014 Martin Hatina <mhatina@redhat.com> - 0.0.1-3
-

* Mon Aug 25 2014 mhatina@redhat.com - 0.0.1-2
- bug fixes, added network tab

