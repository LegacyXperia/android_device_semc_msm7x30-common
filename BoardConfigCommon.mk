# Copyright (C) 2011-2016 The CyanogenMod Project
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

TARGET_SPECIFIC_HEADER_PATH := device/semc/msm7x30-common/include

# Platform
TARGET_BOARD_PLATFORM := msm7x30
TARGET_BOARD_PLATFORM_GPU := qcom-adreno200
USE_CLANG_PLATFORM_BUILD := true

# Architecture
TARGET_ARCH := arm
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_CPU_SMP := false
TARGET_CPU_VARIANT := scorpion

# Qualcomm Hardware
BOARD_USES_QCOM_HARDWARE := true
TARGET_USE_QCOM_BIONIC_OPTIMIZATION := true
TARGET_PROVIDES_LIBLIGHT := true

# Graphics
USE_OPENGL_RENDERER := true
TARGET_USES_ION := true
TARGET_USES_C2D_COMPOSITION := true
TARGET_DISPLAY_INSECURE_MM_HEAP := true
TARGET_NO_ADAPTIVE_PLAYBACK := true
TARGET_NO_INITLOGO := true

# Screenrecord
BOARD_SCREENRECORD_LANDSCAPE_ONLY := true

# Audio
BOARD_HAVE_SEMC_AUDIO := true
BOARD_USES_LEGACY_ALSA_AUDIO := true
AUDIO_FEATURE_ENABLED_INCALL_MUSIC := false
AUDIO_FEATURE_ENABLED_COMPRESS_VOIP := false
AUDIO_FEATURE_ENABLED_PROXY_DEVICE := false

# GPS
BOARD_VENDOR_QCOM_GPS_LOC_API_AMSS_VERSION := 50000
USE_DEVICE_SPECIFIC_GPS := true

# Bluetooth
BOARD_HAVE_BLUETOOTH := true

# Camera
TARGET_NEEDS_NONPIE_CAMERASERVER := true
TARGET_NEEDS_PLATFORM_TEXT_RELOCATIONS := true
BOARD_GLOBAL_CFLAGS += -DSEMC_ICS_CAMERA_BLOB

# Combined root
BOARD_CANT_BUILD_RECOVERY_FROM_BOOT_PATCH := true
BOARD_USES_FULL_RECOVERY_IMAGE := true
BOARD_CUSTOM_BOOTIMG_MK := device/semc/msm7x30-common/boot/custombootimg.mk
TARGET_NO_SEPARATE_RECOVERY := true
TARGET_RECOVERY_PRE_COMMAND := "/sbin/pre-recovery.sh"
TARGET_RELEASETOOLS_EXTENSIONS := device/semc/msm7x30-common

# Init
TARGET_INIT_VENDOR_LIB := libinit_semc

# Recovery
TARGET_RECOVERY_PIXEL_FORMAT := "RGBX_8888"
TARGET_RECOVERY_FSTAB := device/semc/msm7x30-common/rootdir/fstab.semc
TARGET_USE_CUSTOM_LUN_FILE_PATH := "/sys/devices/platform/msm_hsusb/gadget/lun%d/file"
TARGET_RECOVERY_DEVICE_DIRS += device/semc/msm7x30-common
TARGET_RECOVERY_DEVICE_MODULES := libinit_semc

# Kernel
TARGET_NO_KERNEL := false
TARGET_KERNEL_SOURCE := kernel/semc/msm7x30
ifeq ($(HOST_OS),linux)
  KERNEL_TOOLCHAIN := $(ANDROID_BUILD_TOP)/prebuilts/gcc/linux-x86/arm/arm-eabi-4.7/bin
else
  KERNEL_TOOLCHAIN := $(ANDROID_BUILD_TOP)/prebuilts/gcc/darwin-x86/arm/arm-eabi-4.7/bin
endif
TARGET_KERNEL_CROSS_COMPILE_PREFIX := arm-eabi-
BOARD_KERNEL_CMDLINE := # This is ignored by sony's bootloader
BOARD_KERNEL_BASE := 0x00200000
BOARD_KERNEL_PAGESIZE := 131072

# Images
TARGET_NO_BOOTLOADER := true
TARGET_NO_RADIOIMAGE := true
TARGET_USERIMAGES_USE_YAFFS := true
TARGET_USERIMAGES_USE_EXT4 := true

# Partitions
BOARD_SYSTEMIMAGE_FILE_SYSTEM_TYPE := yaffs2
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 964165632
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := yaffs2
BOARD_CACHEIMAGE_PARTITION_SIZE := 4194304
BOARD_FLASH_BLOCK_SIZE := 262144

# Don't generate block mode update zips
BLOCK_BASED_OTA := false

# Bionic
MALLOC_SVELTE := true

# Radio
LINKER_NON_PIE_EXECUTABLES_HEADER_DIR := $(TARGET_SPECIFIC_HEADER_PATH)
BOARD_PROVIDES_LIBRIL := true

# Boot Animation
TARGET_BOOTANIMATION_PRELOAD := true
TARGET_BOOTANIMATION_TEXTURE_CACHE := true

# Sensors
SOMC_CFG_SENSORS := true
SOMC_CFG_SENSORS_COMPASS_AK8975 := yes
SOMC_CFG_SENSORS_LIGHT_AS3676 := yes
SOMC_CFG_SENSORS_LIGHT_AS3676_PATH := "/sys/devices/i2c-0/0-0040"

# CM Hardware tunables
BOARD_HARDWARE_CLASS := device/semc/msm7x30-common/cmhw
