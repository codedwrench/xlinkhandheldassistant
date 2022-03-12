#!/bin/bash
SCRIPT_PWD=`dirname $(readlink -f $0)`
WIFI_ADAPTER=wlp3s0
USER=`whoami`

# Fix Wi-Fi adapter
pkexec env USER=$USER SCRIPT_PWD=$SCRIPT_PWD WIFI_ADAPTER=$WIFI_ADAPTER /bin/bash -c 'ip link set $WIFI_ADAPTER down; \
	$SCRIPT_PWD/set_interface_bss.sh $WIFI_ADAPTER; \
	sudo -u $USER $SCRIPT_PWD/../xlinkhandheldassistant; \
	$SCRIPT_PWD/restore_managed.sh $WIFI_ADAPTER'
