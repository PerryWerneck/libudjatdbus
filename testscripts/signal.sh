#!/bin/bash
echo Sending com.example.signal.hello
dbus-send --session --type=signal / com.example.signal.hello string:"hello"

