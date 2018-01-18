# Create at 2011/05/06 by yanjie_yang

# Common support (must be linked before board specific support)
EXTRA_CFLAGS += -I$(AUTOCONF_DIR)
obj-y += clock.o devices.o irq.o map.o time.o bootmem.o gpio.o

# Specific board support
obj-$(CONFIG_MACH_SN98600) += sn98600.o
obj-$(CONFIG_MACH_SN98601) += sn98600.o
obj-$(CONFIG_MACH_SN98610) += sn98600.o
obj-$(CONFIG_MACH_SN98660) += sn98600.o
