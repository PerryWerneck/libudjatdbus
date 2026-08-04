#!/bin/bash
#
# https://stackoverflow.com/questions/48648952/set-get-property-using-dbus-send
#

dbus-send \
	--session \
	--dest=br.eti.werneck.udjatdbus \
	--print-reply \
	"/dbus" \
	"org.freedesktop.DBus.Properties.GetAll" \
	string:br.eti.werneck.udjatdbus.module



