### Watchdog ###

* Please make sure the WDT driver has been probed first.

 modprobe snx_wdt


#### Watchdog timer example ####
* timeout reboot support
* timeout setting 
* keepalive setting

##### Usage #####

./snx_watchdog_test -h
Usage:
	./snx_watchdog_test [1|2|3] 
	Options
	1 keepalive test 
	2 reset test 
	3 get info test 

