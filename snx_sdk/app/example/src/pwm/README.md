### PWM Example code ###
PWM has two instance, PWM1 and PWM2. They can be used for PWM function or GPIO function.
The example describes how to control PWM pins in PWM function or GPIO function.

#### PWM control ####
* snx_pwm_period_ctl id duty period
	- id 0: pwm1 / 1: pwm2
	- duty (unit: milliseconds)
	- period: period time (unit: milliseconds)


#### PWM GPIO control ####
* snx_pwm_gpio_ctl
	- GPIO Function control example
    ./snx_pwm_gpio_ctl -h
	Usage: ./snx_pwm_gpio_ctl [options]
	Options:
		-h Print this message
		-n GPIO number 0:pwm1 1:pwm2
		-m mode 1:output 0:input
		-v if output, 1:high 0:low

