PWD             := $(shell pwd)
SOURCE		?= $(PWD)/target
FS_DIR		?= $(PWD)/root
DIST_DIR	?= ../image

MKCRAMFS	?= mkcramfs
STRIP		?= arm-linux-strip

export PATH

.PHONY: all install dist clean distclean

all:
	@ if [ -z "$(CROSSTOOL_DIR)" ]; then \
		echo "Error! Please set crosstool directory by 'CROSSTOOL_DIR' variable"; \
		exit 1; \
	fi;
	@ if [ -z "$(MIDDLEWARE_INS_DIR)" ]; then \
		echo "Error! Please set middleware install directory by 'MIDDLEWARE_INS_DIR' variable"; \
		exit 2; \
	fi;
	@ if ! [ -d $(RESCUE_FS_DIR) ]; then \
		echo "Error! Please set filesystem directory by 'RESCUE_FS_DIR' variable"; \
		exit 3; \
	fi
	@ if [ -f $(RESCUE_FS_DIR)/linuxrc ]; then \
		rm -rf $(RESCUE_FS_DIR)/linuxrc; \
	fi
	@ cp -af $(SOURCE)/* $(RESCUE_FS_DIR)
	@ if ! [ -d $(RESCUE_FS_DIR)/lib ]; then \
		install -d $(RESCUE_FS_DIR)/lib; \
	fi
	install -d $(RESCUE_FS_DIR)/dev/net
	install -d $(RESCUE_FS_DIR)/dev/pts
	install -d $(RESCUE_FS_DIR)/dev/shm
	install -d $(RESCUE_FS_DIR)/mnt
	install -d $(RESCUE_FS_DIR)/sys
	install -d $(RESCUE_FS_DIR)/boot
	install -d $(RESCUE_FS_DIR)/proc
	install -d $(RESCUE_FS_DIR)/tmp
	install -d $(RESCUE_FS_DIR)/var/adm
	install -d $(RESCUE_FS_DIR)/var/empty
	install -d $(RESCUE_FS_DIR)/var/log
	install -d $(RESCUE_FS_DIR)/var/run
	install -d $(RESCUE_FS_DIR)/var/spool/cron
	@ find $(RESCUE_FS_DIR)/bin -type f -perm /u+x | xargs $(STRIP) --strip-all 2>/dev/null || true
	@ find $(RESCUE_FS_DIR)/sbin -type f -perm /u+x | xargs $(STRIP) --strip-all 2>/dev/null || true
	@ find $(RESCUE_FS_DIR)/usr/bin -type f -perm /u+x | xargs $(STRIP) --strip-all 2>/dev/null || true
	@ find $(RESCUE_FS_DIR)/usr/sbin -type f -perm /u+x | xargs $(STRIP) --strip-all 2>/dev/null || true
	@ ./copy_shared_library $(RESCUE_FS_DIR) $(CROSSTOOL_DIR) $(MIDDLEWARE_INS_DIR)
	@ find $(RESCUE_FS_DIR)/lib/modules -type f -exec $(STRIP) --strip-unneeded {} + 2>/dev/null || true
	@ find $(RESCUE_FS_DIR)/lib -type f -name \*.so\* | xargs $(STRIP) --strip-unneeded 2>/dev/null || true
	@ if [ -f $(RESCUE_FS_DIR)/linuxrc ]; then \
		mv $(RESCUE_FS_DIR)/linuxrc $(RESCUE_FS_DIR)/init; \
	fi

install: 
	@ if ! [ -d $(DIST_DIR) ]; then \
		install -d $(DIST_DIR); \
	fi
	cd $(RESCUE_FS_DIR); find . | cpio -o -H newc > $(DIST_DIR)/initramfs_data.cpio
clean:
	@ if [ -d $(RESCUE_FS_DIR) ]; then \
		rm -rf $(RESCUE_FS_DIR); \
	fi
