#!/bin/bash
make Debug
if [ "$?" != "0" ]; then
	exit -1
fi

LIBDIR=$(pkg-config --variable libdir libudjat)
MODULEDIR=$(pkg-config --variable module_path libudjat)

sudo ln -sf $(readlink -f .bin/Debug/lib*.so.*) ${LIBDIR}
if [ "$?" != "0" ]; then
	exit -1
fi

mkdir -p ${MODULEDIR}
sudo ln -sf $(readlink -f .bin/Debug/udjat-module-*.so) ${MODULEDIR}
if [ "$?" != "0" ]; then
	exit -1
fi

sudo ln -sf libudjatdbus.so.2.0 ${LIBDIR}/libudjatdbus.so

cat > .bin/sdk.pc << EOF
prefix=/usr
exec_prefix=/usr
libdir=$(readlink -f .bin/Debug)
includedir=$(readlink -f ./src/include)

Name: udjat-module-dbus
Description: UDJAT D-Bus library
Version: 2.0
Libs: -L$(readlink -f .bin/Debug) -ludjatdbus -ldbus-1
Libs.private:  -L/home/perry/project/udjat/libudjat/.bin/Debug -Wl,-rpath,/home/perry/project/udjat/libudjat/.bin/Debug -ludjat
Cflags: -I$(readlink -f ./src/include)
EOF

sudo ln -sf $(readlink -f .bin/sdk.pc) /usr/lib64/pkgconfig/udjat-dbus.pc

