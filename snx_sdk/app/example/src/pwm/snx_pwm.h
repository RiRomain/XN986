#ifndef SNX_PWM_H
#define SNX_PWM_H

/* device name */
#define PWM_DEVICE  "/dev/pwm"
/* TWO PWM ID PWM1 = 0, PWM2 = 1 */
enum SNX_PWM_ID{
	SNX_PWM_1 = 0,
	SNX_PWM_2,
};
#define PWM_SUCCESS    0
#define PWM_FAIL       1

/** \struct pwm_config_param
 * \brief data struct of duty and period 
 * \n
 * \n period_ns:pwm period time(in nanosecond units)
 * \n duty_ns:pwm duty time(in nanosecond units)
 *
 */
struct pwm_config_param{
	unsigned long duty_ns;
	unsigned long period_ns;
};

#define SONIX_PWM_MAGIC		'p'  /*!< Magic number of the pwm driver */
#define SONIX_PWM_REQUEST		_IOR(SONIX_PWM_MAGIC, 1, unsigned int) /*!< request  a usable pwm device */
#define SONIX_PWM_FREE			_IOW(SONIX_PWM_MAGIC, 2, unsigned int) /*!< free a usable pwm device */
#define SONIX_PWM_ENABLE		_IOW(SONIX_PWM_MAGIC, 3, unsigned int) /*!< enable pwm device */
#define SONIX_PWM_DISABLE		_IOW(SONIX_PWM_MAGIC, 4, unsigned int) /*!< disable pwm device */
#define SONIX_PWM_CONFIG		_IOW(SONIX_PWM_MAGIC, 5, unsigned int) /*!< config pwm device duty and period time */
#define SONIX_PWM_INVERSE		_IOW(SONIX_PWM_MAGIC, 6, unsigned int) /*!< set pwm device inverse */
#define SONIX_PWM_READ		_IOR(SONIX_PWM_MAGIC, 7, unsigned int) /*!< use pwm read mode and read the value */
#endif /* SNX_PWM_H */
