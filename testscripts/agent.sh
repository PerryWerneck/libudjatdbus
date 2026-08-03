#!/bin/bash
#
# https://stackoverflow.com/questions/48648952/set-get-property-using-dbus-send
#

gdbus \
	call \
	--session \
	--dest "br.eti.werneck.udjatdbus" \
	--object-path "/" \
	--method br.eti.werneck.udjatdbus.agent.Get


# dbus-send \
# 	--session \
# 	--dest=br.eti.werneck.udjatdbus \
# 	--print-reply \
# 	"/" \
# 	br.eti.werneck.udjatdbus.agent.Get
