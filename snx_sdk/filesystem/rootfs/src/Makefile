PWD             := $(shell pwd)
SOURCE		?= $(PWD)/target
FS_DIR		?= $(PWD)/root
DIST_DIR	?= ../image

MKCRAMFS	?= mkcramfs
MKJFFS2		?= mkfs.jffs2
STRIP		?= arm-linux-strip

include $(AUTOCONF_DIR)/config/snx_sdk.conf

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
	@ if ! [ -d $(FS_DIR) ]; then \
		echo "Error! Please set filesystem directory by 'FS_DIR' variable"; \
		exit 3; \
	fi
	@ if [ -f $(FS_DIR)/linuxrc ]; then \
		rm -rf $(FS_DIR)/linuxrc; \
	fi
	@ cp -af $(SOURCE)/* $(FS_DIR)
	@ if ! [ -d $(FS_DIR)/lib ]; then \
		install -d $(FS_DIR)/lib; \
	fi
#	@ find $(CROSSTOOL_DIR)/arm-linux/sysroot/lib -name \*.so\* | xargs -I {} cp -af {} $(FS_DIR)/lib
#	@ find $(CROSSTOOL_DIR)/arm-linux/sysroot/usr/lib -name \*.so\* | xargs -I {} cp -af {} $(FS_DIR)/lib
#	@ find $(MIDDLEWARE_INS_DIR)/lib -name \*.so\* | xargs -I {} cp -af {} $(FS_DIR)/lib
	@ find $(FS_DIR)/bin -type f -perm /u+x | xargs $(STRIP) --strip-all 2>/dev/null || true
	@ find $(FS_DIR)/sbin -type f -perm /u+x | xargs $(STRIP) --strip-all 2>/dev/null || true
	@ find $(FS_DIR)/usr/bin -type f -perm /u+x | xargs $(STRIP) --strip-all 2>/dev/null || true
ifeq ($(CONFIG_JFFS2_PART), y) 
	@ find $(FS_DIR)/$(CONFIG_JFFS2_PART_MOUNT)/bin -type f -perm /u+x | xargs $(STRIP) --strip-all 2>/dev/null || true 
endif 
	@ find $(FS_DIR)/usr/sbin -type f -perm /u+x | xargs $(STRIP) --strip-all 2>/dev/null || true
	@ ./copy_shared_library $(FS_DIR) $(CROSSTOOL_DIR) $(MIDDLEWARE_INS_DIR)
	@ find $(FS_DIR)/lib/modules -type f -exec $(STRIP) --strip-unneeded {} + 2>/dev/null || true
	@ find $(FS_DIR)/lib -type f -name \*.so\* | xargs $(STRIP) --strip-unneeded 2>/dev/null || true
ifeq ($(CONFIG_JFFS2_PART), y) 
	@ find $(FS_DIR)/$(CONFIG_JFFS2_PART_MOUNT)/lib -type f -name \*.so\* | xargs $(STRIP) --strip-unneeded 2>/dev/null || true 
endif 
#	@ find $(FS_DIR)/lib -type f -name \*gdb\* | xargs rm -f

install: 
	@ if ! [ -d $(DIST_DIR) ]; then \
		install -d $(DIST_DIR); \
	fi
ifeq ($(CONFIG_JFFS2_PART), y) 
	@ $(MKJFFS2) -l -s 0x1000 -e 0x8000 -p $(CONFIG_JFFS2_PART_SIZE)*0x8000 -d $(FS_DIR)/$(CONFIG_JFFS2_PART_MOUNT) -o $(DIST_DIR)/fs.jffs2 
	@ mv $(FS_DIR)/$(CONFIG_JFFS2_PART_MOUNT) $(PWD)/.tmp_app 
	@ mkdir $(FS_DIR)/$(CONFIG_JFFS2_PART_MOUNT) 
endif 
	@ $(MKCRAMFS) -D device_table.txt -q -l $(FS_DIR) $(DIST_DIR)/rootfs.cramfs
ifeq ($(CONFIG_JFFS2_PART), y) 
	@ rm -rf $(FS_DIR)/$(CONFIG_JFFS2_PART_MOUNT) 
	@ mv $(PWD)/.tmp_app $(FS_DIR)/$(CONFIG_JFFS2_PART_MOUNT) 
endif 

clean:
	@ if [ -d $(FS_DIR) ]; then \
		rm -rf $(FS_DIR); \
	fi
