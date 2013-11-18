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
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_CPU_VARIANT := cortex-a8
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_ARCH_LOWMEM := true

# QCOM Hardware
BOARD_USES_QCOM_HARDWARE := true
COMMON_GLOBAL_CFLAGS += -DQCOM_HARDWARE

# Display
TARGET_QCOM_DISPLAY_VARIANT := legacy
TARGET_QCOM_MEDIA_VARIANT := legacy
BOARD_EGL_NEEDS_LEGACY_FB := true
USE_OPENGL_RENDERER := true
TARGET_USES_ION := false
TARGET_USES_C2D_COMPOSITION := true
TARGET_NO_HW_VSYNC := true
BOARD_EGL_CFG := device/semc/msm7x30-common/rootdir/system/etc/egl.cfg
TARGET_NO_INITLOGO := true

# QCOM enhanced A/V
TARGET_ENABLE_QC_AV_ENHANCEMENTS := true

# Changes needed for kitkat
TARGET_RUNNING_WITHOUT_SYNC_FRAMEWORK := true

# Audio
TARGET_QCOM_AUDIO_VARIANT := caf
BOARD_USES_GENERIC_AUDIO := false
BOARD_USES_QCOM_AUDIO_RESETALL := true
BOARD_USES_QCOM_AUDIO_VOIPMUTE := true

# GPS
BOARD_USES_QCOM_GPS := true
BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE := msm7x30
BOARD_VENDOR_QCOM_GPS_LOC_API_AMSS_VERSION := 50000

# Bluetooth
BOARD_HAVE_BLUETOOTH := true

# Webkit
TARGET_FORCE_CPU_UPLOAD := true
ENABLE_WEBGL := true

# Camera
TARGET_DISABLE_ARM_PIE := true
BOARD_NEEDS_MEMORYHEAPPMEM := true
COMMON_GLOBAL_CFLAGS += -DDISABLE_HW_ID_MATCH_CHECK
COMMON_GLOBAL_CFLAGS += -DSEMC_ICS_CAMERA_BLOB -DSEMC_CAMERA_HARDWARE

# Recovery
TARGET_RECOVERY_PIXEL_FORMAT := "RGBX_8888"
BOARD_CUSTOM_BOOTIMG_MK := device/semc/msm7x30-common/custombootimg.mk
TARGET_RECOVERY_PRE_COMMAND := "touch /cache/recovery/boot;sync;"
TARGET_RECOVERY_FSTAB := device/semc/msm7x30-common/rootdir/fstab.semc
BOARD_CUSTOM_RECOVERY_KEYMAPPING := ../../device/semc/msm7x30-common/recovery/recovery_keys.c
TARGET_USE_CUSTOM_LUN_FILE_PATH := "/sys/devices/platform/msm_hsusb/gadget/lun%d/file"

# Kernel
TARGET_KERNEL_SOURCE := kernel/semc/msm7x30
BOARD_KERNEL_CMDLINE := # This is ignored by sony's bootloader
BOARD_KERNEL_BASE := 0x00200000
BOARD_KERNEL_PAGESIZE := 131072
TARGET_NO_BOOTLOADER := true
TARGET_NO_RADIOIMAGE := true
COMMON_GLOBAL_CFLAGS += -DHAS_SEMC_BOOTLOADER

# Boot Animation
TARGET_BOOTANIMATION_PRELOAD := true
TARGET_BOOTANIMATION_TEXTURE_CACHE := true

# Sensors
SOMC_CFG_SENSORS := true
SOMC_CFG_SENSORS_COMPASS_AK8975 := yes
SOMC_CFG_SENSORS_LIGHT_AS3676 := yes
SOMC_CFG_SENSORS_LIGHT_AS3676_PATH := "/sys/devices/i2c-0/0-0040"
SOMC_CFG_SENSORS_LIGHT_AS3676_MAXRANGE := 9000

# A custom ota package maker for a device without an exposed boot partition
TARGET_RELEASETOOL_OTA_FROM_TARGET_SCRIPT := device/semc/msm7x30-common/releasetools/semc_ota_from_target_files

# Workaround to avoid issues with legacy liblights on QCOM platforms
TARGET_PROVIDES_LIBLIGHT := true

# CM Hardware tunables
BOARD_HARDWARE_CLASS := device/semc/msm7x30-common/cmhw
