#
# Regular cron jobs for the pixelann package
#
0 4	* * *	root	[ -x /usr/bin/pixelann_maintenance ] && /usr/bin/pixelann_maintenance
