#
# spec file for package udjat-module-dbus
#
# Copyright (c) 2015 SUSE LINUX GmbH, Nuernberg, Germany.
# Copyright (C) <2008> <Banco do Brasil S.A.>
#
# All modifications and additions to the file contributed by third parties
# remain the property of their copyright owners, unless otherwise agreed
# upon. The license for this file, and modifications and additions to the
# file, is the same license as for the pristine package itself (unless the
# license for the pristine package is not an Open Source License, in which
# case the license is the MIT License). An "Open Source License" is a
# license that conforms to the Open Source Definition (Version 1.9)
# published by the Open Source Initiative.

# Please submit bugfixes or comments via http://bugs.opensuse.org/
#

Summary:		DBus client/server module for udjat
Name:			udjat-module-dbus
Version:		1.0
Release:		0
License:		LGPL-3.0
Source:			%{name}-%{version}.tar.xz

URL:			https://github.com/PerryWerneck/udjat-module-dbus

Group:			Development/Libraries/C and C++
BuildRoot:		/var/tmp/%{name}-%{version}

BuildRequires:	autoconf >= 2.61
BuildRequires:	automake
BuildRequires:	libtool
BuildRequires:	binutils
BuildRequires:	coreutils
BuildRequires:	gcc-c++

BuildRequires:	pkgconfig(libudjat)
BuildRequires:	pkgconfig(dbus-1)

%define MAJOR_VERSION %(echo %{version} | cut -d. -f1)
%define MINOR_VERSION %(echo %{version} | cut -d. -f2 | cut -d+ -f1)
%define _libvrs %{MAJOR_VERSION}_%{MINOR_VERSION}

Requires:		libudjatdbus%{_libvrs} = %{version}

%description
D-Bus client/server module for udjat

#---[ Library ]-------------------------------------------------------------------------------------------------------

%package -n libudjatdbus%{_libvrs}
Summary:	UDJat core library

%description -n libudjatdbus%{_libvrs}
D-Bus client/server library for udjat

Simple D-Bus abstraction library for udjat

#---[ Development ]---------------------------------------------------------------------------------------------------

%package -n udjat-dbus-devel
Summary:	Development files for %{name}
Requires:	pkgconfig(libudjat)
Requires:	pkgconfig(dbus-1)
Requires:	libudjatdbus%{_libvrs} = %{version}

%description -n udjat-dbus-devel

Development files for Udjat's simple abstraction D-Bus library.

#---[ Build & Install ]-----------------------------------------------------------------------------------------------

%prep
%setup

NOCONFIGURE=1 \
	./autogen.sh

%configure 

%build
make all

%install
%makeinstall

%files
%{_libdir}/udjat-modules/*/*.so

%files -n libudjatdbus%{_libvrs}
%defattr(-,root,root)
%{_libdir}/libudjatdbus.so.%{MAJOR_VERSION}.%{MINOR_VERSION}

%files -n udjat-dbus-devel
%defattr(-,root,root)
%{_includedir}/udjat/tools/*.h
%{_libdir}/*.so
%{_libdir}/*.a
%{_libdir}/pkgconfig/*.pc

%pre -n libudjatdbus%{_libvrs} -p /sbin/ldconfig

%post -n libudjatdbus%{_libvrs} -p /sbin/ldconfig

%postun -n libudjatdbus%{_libvrs} -p /sbin/ldconfig

%changelog

