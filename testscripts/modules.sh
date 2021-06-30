#!/bin/bash
#
# https://stackoverflow.com/questions/48648952/set-get-property-using-dbus-send
#

#dbus-send \
#	--session \
#	--dest=br.com.bb.pw3270.a\
#	--print-reply \
#	"/br/com/bb/tn3270/session" \
#	"org.freedesktop.DBus.Properties.Get" \
#	string:br.com.bb.tn3270.session \
#	string:url

#gdbus \
#	call \
#	--session \
#	--dest "br.eti.werneck.udjat" \
#	--object-path "/" \
#	--method br.eti.werneck.udjat.get

dbus-send \
	--session \
	--dest=br.eti.werneck.udjat \
	--print-reply \
	"/" \
	br.eti.werneck.udjat.info.modules.get
	

