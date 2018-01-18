### Timer ###

#### example descripts ####
* snx_timer_test_module
	- timer test module in kernel space.
	- received commands from user space to operate response work.
	- Usage:
		" insmod snx_timer_test_module.ko "
		" mknod /dev/snx_timer_test c 255 0 "

* snx_timer_test
	- Usage:
	- snx_timer_test [1|2|3]
	- Options
		1 timer_measure_test
		2 timer_alarm_test
		3 timer_measure_and_alarm_test
		4 timer_cpu_clock_test

  
