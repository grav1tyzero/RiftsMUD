#!/bin/bash
while :
do
	cd /home/parnell/mud/mudlib
	/usr/bin/driver /home/parnell/mud/mudlib/config.darke
    echo "Press [CTRL+C] to stop.."
	sleep 5s
done
