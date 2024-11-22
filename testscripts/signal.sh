#!/bin/bash
echo Sending com.example.signal.hello
message=${1}
if [ "${message}" == "" ]; then
	message="hello"
fi

dbus-send --session --type=signal / com.example.signal.hello string:"${message}"

