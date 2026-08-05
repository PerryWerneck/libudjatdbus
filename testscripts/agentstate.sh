#!/bin/bash
#
# https://stackoverflow.com/questions/48648952/set-get-property-using-dbus-send
#

dbus-send \
	--session \
	--dest=br.eti.werneck.udjatdbus \
	--print-reply \
	"/" \
	br.eti.werneck.udjatdbus.agent.state
	

