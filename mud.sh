#!/bin/bash
while :
do
	cd /home/parnell/mud/mudlib
	# redirect so we can see it in the mud where we administrate things.
	/usr/bin/driver /home/parnell/mud/mudlib/config.darke > /home/parnell/mud/mudlib/log/driver # -d -fTEST1 -fTEST2
    echo "Press [CTRL+C] to stop.."
	sleep 5s
done
