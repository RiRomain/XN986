### I2C Control ###

#### I2C example ####
* I2C-0 / I2C-1 support
* Read / Write function support
* 8 / 16 bit register / data support


##### Usage #####
./snx_i2c_ctl -h

Usage: ./snx_i2c_ctl
Options:
	-D I2C device 0: i2c-0 1: i2c-1
	-C cmd 0:write 1:read 
	-a address
	-r register
	-v value 
	-m operation mode 
   		0: SNX_I2C_R8D8_MODE 
   		1: SNX_I2C_R8D16_MODE 
   		2: SNX_I2C_R16D8_MODE 
   		3: SNX_I2C_R16D16_MODE 
