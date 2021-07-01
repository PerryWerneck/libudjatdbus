#!/bin/bash
#
# https://stackoverflow.com/questions/48648952/set-get-property-using-dbus-send
#

gdbus \
	call \
	--session \
	--dest "br.eti.werneck.udjat" \
	--object-path "/" \
	--method br.eti.werneck.udjat.agent.get

#dbus-send \
#	--session \
#	--dest=br.eti.werneck.udjat \
#	--print-reply \
#	"/" \
#	br.eti.werneck.udjat.agent.get
	

