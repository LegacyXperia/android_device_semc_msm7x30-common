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

# WARNING: This line must come *before* including the proprietary
# variant, so that it gets overwritten by the parent (which goes
# against the traditional rules of inheritance).
USE_CAMERA_STUB := true

# inherit from the proprietary version
-include vendor/semc/msm7x30-common/BoardConfigVendor.mk

TARGET_SPECIFIC_HEADER_PATH := device/semc/msm7x30-common/include

# Platform
TARGET_BOARD_PLATFORM := msm7x30
TARGET_BOARD_PLATFORM_GPU := qcom-adreno200

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
HWUI_COMPILE_FOR_PERF := true

# Screenrecord
BOARD_SCREENRECORD_LANDSCAPE_ONLY := true

# Audio
BOARD_HAVE_SEMC_AUDIO := true
BOARD_USES_LEGACY_ALSA_AUDIO := true
AUDIO_FEATURE_ENABLED_INCALL_MUSIC := false
AUDIO_FEATURE_ENABLED_COMPRESS_VOIP := false
AUDIO_FEATURE_ENABLED_PROXY_DEVICE := false

# GPS
BOARD_USES_QCOM_GPS := true
BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE := msm7x30
BOARD_VENDOR_QCOM_GPS_LOC_API_AMSS_VERSION := 50000
TARGET_GPS_HAL_PATH := device/qcom/msm7x30-common/gps

# Bluetooth
BOARD_HAVE_BLUETOOTH := true

# Camera
TARGET_DISABLE_ARM_PIE := true
BOARD_NEEDS_MEMORYHEAPPMEM := true
COMMON_GLOBAL_CFLAGS += -DSEMC_ICS_CAMERA_BLOB -DREFBASE_JB_MR1_COMPAT_SYMBOLS
TARGET_RELEASE_CPPFLAGS += -DNEEDS_VECTORIMPL_SYMBOLS

# Combined root
BOARD_CANT_BUILD_RECOVERY_FROM_BOOT_PATCH := true
BOARD_CUSTOM_BOOTIMG := true
BOARD_CUSTOM_BOOTIMG_MK := device/semc/msm7x30-common/custombootimg.mk
TARGET_NO_SEPARATE_RECOVERY := true
TARGET_RECOVERY_PRE_COMMAND := "/sbin/pre-recovery.sh"
TARGET_RELEASETOOLS_EXTENSIONS := device/semc/msm7x30-common

# Recovery
TARGET_RECOVERY_PIXEL_FORMAT := "RGBX_8888"
TARGET_RECOVERY_FSTAB := device/semc/msm7x30-common/rootdir/recovery.fstab
TARGET_USE_CUSTOM_LUN_FILE_PATH := "/sys/devices/platform/msm_hsusb/gadget/lun%d/file"
TARGET_RECOVERY_DEVICE_DIRS += device/semc/msm7x30-common

# Kernel
TARGET_NO_KERNEL := false
TARGET_KERNEL_SOURCE := kernel/semc/msm7x30
ifeq ($(HOST_OS),linux)
  KERNEL_TOOLCHAIN := $(ANDROID_BUILD_TOP)/prebuilts/gcc/linux-x86/arm/arm-eabi-4.7/bin
else
  KERNEL_TOOLCHAIN := $(ANDROID_BUILD_TOP)/prebuilts/gcc/darwin-x86/arm/arm-eabi-4.7/bin
endif
BOARD_KERNEL_CMDLINE := # This is ignored by sony's bootloader
BOARD_KERNEL_BASE := 0x00200000
BOARD_KERNEL_PAGESIZE := 131072

# We don't build bootloader nor radio
TARGET_NO_BOOTLOADER := true
TARGET_NO_RADIOIMAGE := true

# Don't generate block mode update zips
BLOCK_BASED_OTA := false

# Enable dex-preoptimization to speed up first boot sequence
ifeq ($(HOST_OS),linux)
  ifeq ($(TARGET_BUILD_VARIANT),userdebug)
    ifeq ($(WITH_DEXPREOPT),)
      WITH_DEXPREOPT := true
    endif
  endif
endif

# Use dlmalloc instead of jemalloc
MALLOC_IMPL := dlmalloc

# Radio
TARGET_NEEDS_NON_PIE_SUPPORT := true
BOARD_PROVIDES_LIBRIL := true

# Boot Animation
TARGET_BOOTANIMATION_PRELOAD := true
TARGET_BOOTANIMATION_TEXTURE_CACHE := true

# Sensors
SOMC_CFG_SENSORS := true
SOMC_CFG_SENSORS_COMPASS_AK8975 := yes
SOMC_CFG_SENSORS_LIGHT_AS3676 := yes
SOMC_CFG_SENSORS_LIGHT_AS3676_PATH := "/sys/devices/i2c-0/0-0040"
SOMC_CFG_SENSORS_LIGHT_AS3676_MAXRANGE := 13000
SOMC_CFG_SENSORS_LIGHT_AS3676_DISABLE_ALS_SWITCH := yes

# CM Hardware tunables
BOARD_HARDWARE_CLASS := device/semc/msm7x30-common/cmhw
