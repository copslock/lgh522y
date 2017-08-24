#
# Copyright (C) 2009-2011 The Android-x86 Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
ifeq ($(LINUX_KERNEL_VERSION),kernel-3.10)
ifneq ($(strip $(MTK_EMULATOR_SUPPORT)),yes)
ifneq ($(strip $(MTK_PROJECT_NAME)),)

ifneq ($(wildcard $(call my-dir)/arch/$(TARGET_ARCH)/configs/$(KERNEL_DEFCONFIG)),)

KERNEL_DIR := $(call my-dir)
ROOTDIR := $(abspath $(TOP))

KERNEL_OUT := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ

ifeq ($(TARGET_ARCH), arm64)
  ifeq ($(MTK_APPENDED_DTB_SUPPORT), yes)
    TARGET_PREBUILT_KERNEL := $(KERNEL_OUT)/arch/$(TARGET_ARCH)/boot/Image.gz-dtb
  else
    TARGET_PREBUILT_KERNEL := $(KERNEL_OUT)/arch/$(TARGET_ARCH)/boot/Image.gz
  endif
else
  ifeq ($(MTK_APPENDED_DTB_SUPPORT), yes)
    TARGET_PREBUILT_KERNEL := $(KERNEL_OUT)/arch/$(TARGET_ARCH)/boot/zImage-dtb
  else
    TARGET_PREBUILT_KERNEL := $(KERNEL_OUT)/arch/$(TARGET_ARCH)/boot/zImage
  endif
endif
TARGET_PREBUILT_KERNEL_BIN := $(KERNEL_OUT)/arch/$(TARGET_ARCH)/boot/zImage.bin

TARGET_KERNEL_CONFIG := $(KERNEL_OUT)/.config
KERNEL_HEADERS_INSTALL := $(KERNEL_OUT)/usr

ifneq ($(strip $(TARGET_NO_KERNEL)),true)
  INSTALLED_KERNEL_TARGET := $(PRODUCT_OUT)/kernel
else
  INSTALLED_KERNEL_TARGET :=
endif

HW_REV := HW_REV=$(LGE_HW)

ifeq ($(KERNEL_CROSS_COMPILE),)
ifeq ($(TARGET_ARCH), arm64)
KERNEL_CROSS_COMPILE := aarch64-linux-android-
else
KERNEL_CROSS_COMPILE := arm-eabi-
endif
endif

# add to gatord daemon for DS-5 only userdebug & eng mode
KERNEL_MODULES_INSTALL := $(KERNEL_OUT)/lib/modules
KERNEL_MODULES_OUT := $(TARGET_OUT)/lib/modules

define mv-modules
if [ ! -d $(KERNEL_MODULES_OUT) ]; then\
  mkdir -p $(KERNEL_MODULES_OUT);\
fi

mdpath=`find $(KERNEL_MODULES_INSTALL) -type f -name modules.dep`;\
if [ "$$mdpath" != "" ];then\
mpath=`dirname $$mdpath`;\
ko=`find $$mpath/kernel -type f -name *.ko`;\
for i in $$ko; do mv $$i $(KERNEL_MODULES_OUT)/; done;\
fi
endef

$(KERNEL_OUT):
	mkdir -p $@

.PHONY: kernel kernel-defconfig bootchart2defconfig kernel-menuconfig clean-kernel
kernel-menuconfig: | $(KERNEL_OUT)
	$(MAKE) -C $(KERNEL_DIR) O=../$(KERNEL_OUT) ARCH=$(TARGET_ARCH) MTK_TARGET_PROJECT=${MTK_TARGET_PROJECT} CROSS_COMPILE=$(KERNEL_CROSS_COMPILE) ROOTDIR=$(ROOTDIR) menuconfig

kernel-savedefconfig: | $(KERNEL_OUT)
	cp $(TARGET_KERNEL_CONFIG) $(KERNEL_DIR)/arch/$(TARGET_ARCH)/configs/$(KERNEL_DEFCONFIG)

$(TARGET_PREBUILT_KERNEL): kernel
	@echo Done kernel

ifeq ($(INIT_BOOTCHART2),true)
KERNEL_DEFCONFIG_PATH:=$(KERNEL_DIR)/arch/$(TARGET_ARCH)/configs/$(KERNEL_DEFCONFIG)
KERNEL_DEFCONFIG_BC2_PATH:=$(KERNEL_DIR)/arch/$(TARGET_ARCH)/configs/bc2_$(KERNEL_DEFCONFIG)

bootchart2defconfig:
	cp -f $(KERNEL_DEFCONFIG_PATH) $(KERNEL_DEFCONFIG_BC2_PATH)
	echo "CONFIG_TASKSTATS=y" >> $(KERNEL_DEFCONFIG_BC2_PATH)
	echo "CONFIG_TASK_DELAY_ACCT=y" >> $(KERNEL_DEFCONFIG_BC2_PATH)
	echo "CONFIG_TASK_XACCT=y" >> $(KERNEL_DEFCONFIG_BC2_PATH)
	echo "CONFIG_TASK_IO_ACCOUNTING=y" >> $(KERNEL_DEFCONFIG_BC2_PATH)
	echo "CONFIG_CONNECTOR=y" >> $(KERNEL_DEFCONFIG_BC2_PATH)
	echo "CONFIG_PROC_EVENTS=y" >> $(KERNEL_DEFCONFIG_BC2_PATH)

$(TARGET_KERNEL_CONFIG): $(KERNEL_OUT) bootchart2defconfig
	$(MAKE) -C $(KERNEL_DIR) O=../$(KERNEL_OUT) ARCH=$(TARGET_ARCH) MTK_TARGET_PROJECT=${MTK_TARGET_PROJECT} CROSS_COMPILE=$(KERNEL_CROSS_COMPILE) ROOTDIR=$(ROOTDIR) bc2_$(KERNEL_DEFCONFIG)
else
$(TARGET_KERNEL_CONFIG) kernel-defconfig: $(KERNEL_DIR)/arch/$(TARGET_ARCH)/configs/$(KERNEL_DEFCONFIG) | $(KERNEL_OUT)
	$(MAKE) -C $(KERNEL_DIR) O=../$(KERNEL_OUT) ARCH=$(TARGET_ARCH) MTK_TARGET_PROJECT=${MTK_TARGET_PROJECT} CROSS_COMPILE=$(KERNEL_CROSS_COMPILE) ROOTDIR=$(ROOTDIR) $(KERNEL_DEFCONFIG)
endif
	$(MAKE) -C $(KERNEL_DIR) O=../$(KERNEL_OUT) ARCH=$(TARGET_ARCH) MTK_TARGET_PROJECT=${MTK_TARGET_PROJECT} CROSS_COMPILE=$(KERNEL_CROSS_COMPILE) ROOTDIR=$(ROOTDIR) oldconfig

$(KERNEL_HEADERS_INSTALL): $(TARGET_KERNEL_CONFIG) | $(KERNEL_OUT)
	$(MAKE) -C $(KERNEL_DIR) O=../$(KERNEL_OUT) ARCH=$(TARGET_ARCH) MTK_TARGET_PROJECT=${MTK_TARGET_PROJECT} CROSS_COMPILE=$(KERNEL_CROSS_COMPILE) ROOTDIR=$(ROOTDIR) headers_install

kernel: $(TARGET_KERNEL_CONFIG) $(KERNEL_HEADERS_INSTALL) | $(KERNEL_OUT)
	$(MAKE) -C $(KERNEL_DIR) O=../$(KERNEL_OUT) ARCH=$(TARGET_ARCH) MTK_TARGET_PROJECT=${MTK_TARGET_PROJECT} CROSS_COMPILE=$(KERNEL_CROSS_COMPILE) ROOTDIR=$(ROOTDIR)

# add to gatord daemon for DS-5 only userdebug & eng mode, only mc90ds model
ifeq ($(TARGET_PRODUCT), mc90ds_global_com)
    ifneq ($(TARGET_BUILD_VARIANT), user)
	    $(MAKE) -C $(KERNEL_DIR) O=../$(KERNEL_OUT) INSTALL_MOD_PATH=./ ARCH=$(TARGET_ARCH) MTK_TARGET_PROJECT=${MTK_TARGET_PROJECT} CROSS_COMPILE=$(KERNEL_CROSS_COMPILE) ROOTDIR=$(ROOTDIR) modules_install
	    $(mv-modules)
    endif
endif

$(INSTALLED_KERNEL_TARGET): kernel

ifeq ($(strip $(MTK_HEADER_SUPPORT)),yes)
$(TARGET_PREBUILT_KERNEL_BIN): $(TARGET_PREBUILT_KERNEL) | $(HOST_OUT_EXECUTABLES)/mkimage
	$(HOST_OUT_EXECUTABLES)/mkimage $< KERNEL 0xffffffff > $@ 	

$(INSTALLED_KERNEL_TARGET): $(TARGET_PREBUILT_KERNEL_BIN) | $(ACP)
	$(copy-file-to-target)
else
$(INSTALLED_KERNEL_TARGET): $(TARGET_PREBUILT_KERNEL) | $(ACP)
	$(copy-file-to-target)
endif

clean-kernel:
	@rm -rf $(KERNEL_OUT)

endif
endif
endif
endif # Ifeq ($(LINUX_KERNEL_VERSION),kernel-3.10)

