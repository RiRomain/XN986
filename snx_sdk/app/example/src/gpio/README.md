### GPIO ###
* SN9860X support 4 pins GPIO
* SN98610 supports 6 pins GPIO
* SN98660 supports 6 pins GPIO

#### GPIO Control Example ####
* GPIO control utility example
* Support Input or Output mode setting for each pin.
* Support High / Low setting in Output mode

##### GPIO Control Usage #####
./snx_gpio_ctl -h
Usage: ./snx_gpio_ctl [options]/n
Options:
	-h Print this message
	-n GPIO number 0:GPIO[0] 1:GPIO[1] 2:GPIO[2] 3:GPIO[3] (4:GPIO[4] 5:GPIO[5])
	-m mode 1:output 0:input
	-v if output, 1:high 0:low

#### GPIO functions Example ####
* Basic function test example
 - interrupt mode setting example
 - Input / Output mode setting example

##### GPIO functions Usage #####
* Options:
	-h Print this message
	-n GPIO number
	-i Interrupt edge set 0: none 1: rasing 2: falling 3: both
	-d Direction set in or out
	-v Value 1:high 0: low
	-c Do counts
	-t Time(ms)


