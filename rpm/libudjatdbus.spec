#
# spec file for package libudjatdbus
#
# Copyright (c) <2024> Perry Werneck <perry.werneck@gmail.com>.
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via https://github.com/PerryWerneck/libudjatdbus/issues
#

%define module_name dbus

Summary:		DBus client/server library for %{udjat_product_name}  
Name:			libudjat%{module_name}
Version: 2.0.2
Release:		0
License:		LGPL-3.0
Source:			%{name}-%{version}.tar.xz

URL:			https://github.com/PerryWerneck/libudjat%{module_name}

Group:			Development/Libraries/C and C++
BuildRoot:		/var/tmp/%{name}-%{version}

BuildRequires:	gcc-c++ >= 5
BuildRequires:	pkgconfig(dbus-1)
BuildRequires:	pkgconfig(libudjat) >= 2.0
BuildRequires:	pkgconfig(systemd)
BuildRequires:	udjat-rpm-macros
BuildRequires:	meson

%description
DBus client/server library for %{udjat_product_name}

C++ DBus client and server classes for use with lib%{udjat_product_name}

%package -n %{udjat_library}
Summary: DBus client/server library for %{udjat_product_name}

%description -n %{udjat_library}
DBus client/server library for %{udjat_product_name}

C++ DBus client/service classes for use with lib%{udjat_product_name}

%package devel
Summary: Development files for %{name}
%udjat_devel_requires

%description devel
DBus client/server library for %{udjat_product_name}

C++ DBus client/server classes for use with lib%{udjat_product_name}

%udjat_module_package -n %{module_name}

%prep
%autosetup
%meson

%build
%meson_build

%install
%meson_install

%files -n %{udjat_library}
%defattr(-,root,root)
%{_libdir}/%{name}.so.%{udjat_package_major}.%{udjat_package_minor}

%files devel
%defattr(-,root,root)

%{_libdir}/*.so
%{_libdir}/*.a
%{_libdir}/pkgconfig/*.pc

%dir %{_includedir}/udjat/tools/dbus
%{_includedir}/udjat/tools/*.h
%{_includedir}/udjat/tools/dbus/*.h
%{_includedir}/udjat/module/*.h

%dir %{_includedir}/udjat/alert
%{_includedir}/udjat/alert/*.h

%post -n %{udjat_library} -p /sbin/ldconfig

%postun -n %{udjat_library} -p /sbin/ldconfig

%changelog

