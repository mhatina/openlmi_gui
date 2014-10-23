Name:           lmicc-logger
Version:        0.1.1
Release:        1%{?dist}
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
* Thu Oct 23 2014 Martin Hatina <mhatina@redhat.com> - 0.1.1-1
- bug fixes, lot of improvements

* Fri Sep 19 2014 Martin Hatina <mhatina@redhat.com> - 0.1.0-2
- bug fixes

* Thu Sep 18 2014 Martin Hatina <mhatina@redhat.com> - 0.1.0-1
- lot of new features and bug fixes

* Tue Aug 26 2014 Martin Hatina <mhatina@redhat.com> - 0.0.1-3
- plugin path related bug fix

* Mon Aug 25 2014 mhatina@redhat.com - 0.0.1-2
- bug fixes, added network tab

