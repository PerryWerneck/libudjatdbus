#!/bin/bash
#
# https://stackoverflow.com/questions/48648952/set-get-property-using-dbus-send
#

gdbus \
	call \
	--session \
	--dest "br.eti.werneck.udjat.dbustest" \
	--object-path "/" \
	--method br.eti.werneck.udjat.MyInterface.echo \
	"Simple text"
