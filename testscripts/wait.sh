#!/bin/bash
DBUS_INTERFACE="br.eti.werneck.udjat.MyInterface"
DBUS_MEMBER="changed"

echo "Waiting for D-Bus signal: interface=$DBUS_INTERFACE, member=$DBUS_MEMBER"

dbus-monitor --session "type='signal',interface='$DBUS_INTERFACE',member='$DBUS_MEMBER'"

