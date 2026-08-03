#!/bin/bash
#
#

gdbus \
	introspect \
	--session \
	--object-path "/" \
	--dest "br.eti.werneck.udjatdbus"



