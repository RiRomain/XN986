### RTC ###


#### Installation ####
1. please Make sure rtc driver has been installed
   modprobe snx_rtc or insmod snx_rtc.ko


#### RTC control ####

* snx_rtc_test
	- RTC Test Application
	- Usage: ./snx_rtc_test ops
    - ops: 
		0) Exit
		1) snx_rtc_timer_test
		2) snx_rtc_alarm_interrupt_test
		3) snx_rtc_wakeup_interrupt_test
