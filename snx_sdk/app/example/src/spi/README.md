### SPI ###

#### SPI control example ####
* Please make sure NO GPIO function is set to any SPI pins. (Check board-info/sn986xx/gpio/gpio.txt)
* default bitrate is 1Mbps (please modify SPI_MAX_SPEED to change )
* In this example, 8 x 8bit data would be xfered in 1Mbps speed. 
	- you can modify data and bit_per_word to change


#### SPI GPIO Control example ####
* Please check if SPI has been in GPIO mode
* Two SPI instance support
* Input / Output mode support

##### Usage #####

Usage: snx_spi_gpio [options]
Options:
	-h                 Print this message
	-d | --device      /dev/spidev0.0, /dev/spidev1.0
	-p | --pin         clk, cs, tx, rx
	-t | --type        in, out (default: in)
	-v | --value       0 or 1 (default: 0)
