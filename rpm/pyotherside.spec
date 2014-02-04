Summary: Asynchronous Python 3 Bindings for Qt 5
Name: pyotherside-qml-plugin-python3-qt5
Version: 1.0.0
Release: 9
Source: %{name}-%{version}.tar.gz
URL: http://thp.io/2011/pyotherside/
License: ISC
Group: Development/Libraries
BuildRequires: python3-devel
BuildRequires: qt5-qmake
BuildRequires: pkgconfig(Qt5Gui)
BuildRequires: pkgconfig(Qt5Qml)
BuildRequires: pkgconfig(Qt5Quick)
BuildRequires: pkgconfig(Qt5Test)
Requires: python3-base

%description
A QML Plugin that provides access to a Python 3 interpreter from QML.

%prep
%setup -q

%build
%qmake5
make %{?_smp_mflags}

%install
make INSTALL_ROOT=%{buildroot} install

%post
/sbin/ldconfig

%postun
/sbin/ldconfig

%files
%defattr(-,root,root)
%doc README COPYING
%{_libdir}/qt5/qml/io/thp/pyotherside/*
