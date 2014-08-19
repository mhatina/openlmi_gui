Name:           lmicc
Version:        0.0.1
Release:        1%{?dist}
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

%package doc
Summary:        Documentation for %{name}
Group:          Documentation

%description
%{name} is a Graphical User Interface for OpenLMI.

%description doc
%{summary}

%prep
%setup -q -n %{name}-%{version}

%define path %{_libdir}/lmicc

%build
cd src/ui
qmake-qt4 target.path=%{buildroot}%{_bindir} DEFINES+="PLUGIN_PATH=%{path}"
make 
cd -

%install
make -C src/ui install

%files
%defattr(-,root,root,0755)  
%doc LICENSE README.md
%{_bindir}/lmicc 

%files doc
%dir %{_docdir}/%{name}

%changelog
