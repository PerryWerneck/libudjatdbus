#!/bin/bash
dbus-send --session --type=signal / com.example.signal.hello string:"hello"

