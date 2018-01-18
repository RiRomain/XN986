#ifndef _MMC_H
#define _MMC_H
//#ifndef ASMARM_ARCH_MMC_H
//#define ASMARM_ARCH_MMC_H

struct device;
struct mmc_host;

//Nora: s3c24xx_mci_platform_data-->sn926_mci_platform_data
//struct sn926_mci_platform_data {
//Davian: sn926_mci_platform_data-->snx_mci_platform_data
struct snx_mci_platform_data {
  unsigned int ocr_mask;			/* available voltages */
  unsigned long detect_delay;			/* delay in jiffies before detecting cards after interrupt */
  int (*init)(struct device *, irqreturn_t (*)(int, void *, struct pt_regs *), void *);
  int (*get_ro)(struct device *);
  void (*setpower)(struct device *, unsigned int);
  void (*exit)(struct device *, void *);
};


//Nora: s3c24xx_mmc_platdata-->sn926_mmc_platdata
//struct sn926_mmc_platdata {
//Davian: sn926_mmc_platdata-->snx_mmc_platdata
struct snx_mmc_platdata {
  unsigned int	detect;
  unsigned int	wprotect;
  unsigned int	detect_polarity;
  unsigned int	wprotect_polarity;

  unsigned long	f_max;
  unsigned long	ocr_avail;

  void		(*set_power)(unsigned int to);
};

//extern void s3c24xx_set_mci_info (struct s3c24xx_mci_platform_data *info); //Nora: mask
#endif
