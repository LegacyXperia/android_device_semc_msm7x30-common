# Copyright (C) 2011-2013 The CyanogenMod Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

LOCAL_PATH := $(call my-dir)

uncompressed_ramdisk := $(PRODUCT_OUT)/ramdisk.cpio
$(uncompressed_ramdisk): $(INSTALLED_RAMDISK_TARGET)
	gunzip -c $< > $@

recovery_uncompressed_ramdisk := $(PRODUCT_OUT)/ramdisk-recovery.cpio
recovery_uncompressed_device_ramdisk := $(PRODUCT_OUT)/ramdisk-recovery-device.cpio
$(recovery_uncompressed_device_ramdisk): $(MKBOOTFS) \
		$(INTERNAL_RECOVERYIMAGE_FILES) \
		$(recovery_initrc) $(recovery_sepolicy) $(recovery_kernel) \
		$(INSTALLED_2NDBOOTLOADER_TARGET) \
		$(recovery_build_prop) $(recovery_resource_deps) $(recovery_root_deps) \
		$(recovery_fstab) \
		$(RECOVERY_INSTALL_OTA_KEYS) \
		$(INTERNAL_BOOTIMAGE_FILES)
	$(call build-recoveryramdisk)
	@echo "----- Making uncompressed recovery ramdisk ------"
	$(hide) $(MKBOOTFS) $(TARGET_RECOVERY_ROOT_OUT) > $@
	$(hide) rm -f $(recovery_uncompressed_ramdisk)
	$(hide) cp $(recovery_uncompressed_device_ramdisk) $(recovery_uncompressed_ramdisk)

recovery_ramdisk := $(PRODUCT_OUT)/ramdisk-recovery.img
$(recovery_ramdisk): $(MINIGZIP) \
		$(recovery_uncompressed_device_ramdisk)
	@echo "----- Making compressed recovery ramdisk ------"
	$(hide) $(MINIGZIP) < $(recovery_uncompressed_ramdisk) > $@

INITSEMC := $(PRODUCT_OUT)/utilities/init_semc

INSTALLED_BOOTIMAGE_TARGET := $(PRODUCT_OUT)/boot.img
$(INSTALLED_BOOTIMAGE_TARGET): $(PRODUCT_OUT)/kernel \
		$(uncompressed_ramdisk) \
		$(recovery_uncompressed_device_ramdisk) \
		$(INSTALLED_RAMDISK_TARGET) \
		$(INITSEMC) \
		$(PRODUCT_OUT)/utilities/toybox \
		$(PRODUCT_OUT)/utilities/keycheck \
		$(MKBOOTIMG) $(MINIGZIP) \
		$(INTERNAL_BOOTIMAGE_FILES)
	@echo "----- Making boot image ------"

	$(hide) rm -fr $(PRODUCT_OUT)/combinedroot
	$(hide) cp -a $(PRODUCT_OUT)/root $(PRODUCT_OUT)/combinedroot
	$(hide) mkdir -p $(PRODUCT_OUT)/combinedroot/sbin

	$(hide) mv $(PRODUCT_OUT)/root/initlogo.rle $(PRODUCT_OUT)/combinedroot/initlogo.rle
	$(hide) cp $(recovery_uncompressed_ramdisk) $(PRODUCT_OUT)/combinedroot/sbin/
	$(hide) cp $(PRODUCT_OUT)/utilities/keycheck $(PRODUCT_OUT)/combinedroot/sbin/
	$(hide) cp $(PRODUCT_OUT)/utilities/toybox $(PRODUCT_OUT)/combinedroot/sbin/toybox_init

	$(hide) cp $(INITSEMC) $(PRODUCT_OUT)/combinedroot/sbin/init_semc
	$(hide) chmod 755 $(PRODUCT_OUT)/combinedroot/sbin/init_semc
	$(hide) mv $(PRODUCT_OUT)/combinedroot/init $(PRODUCT_OUT)/combinedroot/init.real
	$(hide) ln -s sbin/init_semc $(PRODUCT_OUT)/combinedroot/init

	$(hide) $(MKBOOTFS) $(PRODUCT_OUT)/combinedroot/ > $(PRODUCT_OUT)/combinedroot.cpio
	$(hide) cat $(PRODUCT_OUT)/combinedroot.cpio | $(MINIGZIP) -9 > $(PRODUCT_OUT)/combinedroot.fs

	$(hide) $(MKBOOTIMG) --kernel $(PRODUCT_OUT)/kernel --ramdisk $(PRODUCT_OUT)/combinedroot.fs --base $(BOARD_KERNEL_BASE) --pagesize $(BOARD_KERNEL_PAGESIZE) -o $(INSTALLED_BOOTIMAGE_TARGET)
	@echo "Made boot image: $@"

INSTALLED_RECOVERYIMAGE_TARGET := $(PRODUCT_OUT)/recovery.img
$(INSTALLED_RECOVERYIMAGE_TARGET): $(HOST_OUT_EXECUTABLES)/imgdiff $(MKBOOTIMG) \
		$(recovery_ramdisk) \
		$(recovery_kernel)
	@echo "----- Making recovery image ------"
	$(hide) $(MKBOOTIMG) --kernel $(PRODUCT_OUT)/kernel --ramdisk $(PRODUCT_OUT)/ramdisk-recovery.img --base $(BOARD_KERNEL_BASE) --pagesize $(BOARD_KERNEL_PAGESIZE) -o $(INSTALLED_RECOVERYIMAGE_TARGET)
	@echo "Made recovery image: $@"
