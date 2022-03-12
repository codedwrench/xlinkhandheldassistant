#!/bin/bash
SCRIPT_PWD=`dirname $(readlink -f $0)`
x-terminal-emulator -e "$SCRIPT_PWD/start-xlinkhandheldassistant-cli.sh"
