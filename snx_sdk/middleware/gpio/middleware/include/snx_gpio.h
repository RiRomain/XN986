#ifndef SNX_GPIO_H
#define SNX_GPIO_H
//#include "../../buildscript/include/generated/snx_gpio_conf.h"
#include "snx_gpio_conf.h"
// function return vaule
#define GPIO_SUCCESS 0
#define GPIO_FAIL 1
#include "snx_sdk_conf.h"
#ifdef __cplusplus
extern "C" {
#endif

// common structure for total gpio function
typedef struct {
  unsigned int  pinumber; //pin number 
  unsigned int  mode; // 0: input 1: output
  unsigned int  value; // 0:low 1:high  
}gpio_pin_info;

//gpio pin
#define GPIO_PIN_0  0
#define GPIO_PIN_1  1
#define GPIO_PIN_2  2
#define GPIO_PIN_3  3
#define GPIO_PIN_4  4  //only sn98610
#define GPIO_PIN_5  5  //only sn98610

// gpio interrupt mode  only gpio support
#define INTURREPT_NONE      0
#define INTURREPT_RISING    1
#define INTURREPT_FALLING   2 
#define INTURREPT_BOTH      3 

// pwm gpio pin
#define PWM_GPIO_PIN0 0
#define PWM_GPIO_PIN1 1

//spi gpio pin
#define SPI_GPIO_CLK_PIN  0
#define SPI_GPIO_FS_PIN		1
#define SPI_GPIO_TX_PIN		2
#define SPI_GPIO_RX_PIN		3

// ms1_gpio pin  
#define MS1_GPIO_PIN1 1
#define MS1_GPIO_PIN2 2
#define MS1_GPIO_PIN3 3
#define MS1_GPIO_PIN4 4
#define MS1_GPIO_PIN5 5
#define MS1_GPIO_PIN6 6
#define MS1_GPIO_PIN7 7
#define MS1_GPIO_PIN11 11
#define MS1_GPIO_PIN12 12
#define MS1_GPIO_PIN13 13
#define MS1_GPIO_PIN14 14



// gpio function
int snx_gpio_open();
int snx_gpio_close();
int snx_gpio_write(gpio_pin_info info);
int snx_gpio_read(gpio_pin_info* info);
int snx_gpio_set_interrupt(int pin, int type); //only input mode
int snx_gpio_poll (int pin,int timeout); // only input mode timeout unit is ms
// ms1_io gpio function
int snx_ms1_gpio_open();
int snx_ms1_gpio_close ();
int snx_ms1_gpio_write (gpio_pin_info info);
int snx_ms1_gpio_read (gpio_pin_info* info);
// pwm gpio function 
int snx_pwm_gpio_open();
int snx_pwm_gpio_close ();
int snx_pwm_gpio_write (gpio_pin_info info);
int snx_pwm_gpio_read (gpio_pin_info* info);
// spi gpio function
int snx_spi_gpio_open ();
int snx_spi_gpio_close ();
int snx_spi_gpio_write (gpio_pin_info info);
int snx_spi_gpio_read (gpio_pin_info* info);

#ifdef __cplusplus
}
#endif

#endif /* SNX_GPIO_H */
