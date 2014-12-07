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

# These is the hardware-specific overlay, which points to the location
# of hardware-specific resource overrides, typically the frameworks and
# application settings that are stored in resourced.
DEVICE_PACKAGE_OVERLAYS += device/semc/msm7x30-common/overlay

COMMON_PATH := device/semc/msm7x30-common

$(call inherit-product, frameworks/native/build/phone-hdpi-512-dalvik-heap.mk)

$(call inherit-product, device/common/gps/gps_eu_supl.mk)

PRODUCT_BOOT_JARS += qcmediaplayer

# These are the common hardware-specific features
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/handheld_core_hardware.xml:system/etc/permissions/handheld_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:system/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/etc/permissions/android.hardware.usb.host.xml

# Init scripts
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/rootdir/init.semc.rc:root/init.semc.rc \
    $(COMMON_PATH)/rootdir/ueventd.semc.rc:root/ueventd.semc.rc

# Reboot to recovery related scripts
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/rootdir/sbin/pre-recovery.sh:root/sbin/pre-recovery.sh

# fstab
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/rootdir/fstab.semc:root/fstab.semc

# Common device specific configs
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/rootdir/system/etc/audio_policy.conf:system/etc/audio_policy.conf \
    $(COMMON_PATH)/rootdir/system/etc/media_profiles.xml:system/etc/media_profiles.xml

# Media codecs
PRODUCT_COPY_FILES += \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:system/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_telephony.xml:system/etc/media_codecs_google_telephony.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video.xml:system/etc/media_codecs_google_video.xml \
    frameworks/av/media/libstagefright/data/media_codecs_ffmpeg.xml:system/etc/media_codecs_ffmpeg.xml \
    $(COMMON_PATH)/rootdir/system/etc/media_codecs.xml:system/etc/media_codecs.xml

# Common keylayouts
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/rootdir/system/usr/keylayout/msm_pmic_pwr_key.kl:system/usr/keylayout/msm_pmic_pwr_key.kl \
    $(COMMON_PATH)/rootdir/system/usr/keylayout/simple_remote.kl:system/usr/keylayout/simple_remote.kl \
    $(COMMON_PATH)/rootdir/system/usr/keylayout/simple_remote_appkey.kl:system/usr/keylayout/simple_remote_appkey.kl

# Offline charging animation
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/animations/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/charging_animation_01.png:system/semc/chargemon/data/charging_animation_01.png \
    $(COMMON_PATH)/animations/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/charging_animation_02.png:system/semc/chargemon/data/charging_animation_02.png \
    $(COMMON_PATH)/animations/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/charging_animation_03.png:system/semc/chargemon/data/charging_animation_03.png \
    $(COMMON_PATH)/animations/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/charging_animation_04.png:system/semc/chargemon/data/charging_animation_04.png \
    $(COMMON_PATH)/animations/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/charging_animation_05.png:system/semc/chargemon/data/charging_animation_05.png \
    $(COMMON_PATH)/animations/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/charging_animation_06.png:system/semc/chargemon/data/charging_animation_06.png \
    $(COMMON_PATH)/animations/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/charging_animation_07.png:system/semc/chargemon/data/charging_animation_07.png

# Boot logo
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT).rle:root/initlogo.rle

# Audio
PRODUCT_PACKAGES += \
    audio.a2dp.default \
    audio.usb.default \
    audio.r_submix.default \
    audio.primary.msm7x30 \
    libaudio-resampler \
    libaudioparameter \
    libdashplayer

# Graphics
PRODUCT_PACKAGES += \
    copybit.msm7x30 \
    gralloc.msm7x30 \
    hwcomposer.msm7x30 \
    memtrack.msm7x30 \
    libgenlock \
    libmemalloc \
    liboverlay \
    libqdutils \
    libtilerenderer

# Hal
PRODUCT_PACKAGES += \
    power.msm7x30 \
    gps.msm7x30 \
    lights.msm7x30 \
    sensors.msm7x30

# QCOM OMX
PRODUCT_PACKAGES += \
    libstagefrighthw \
    libOmxCore \
    libOmxVdec \
    libOmxVenc \
    libc2dcolorconvert

# qcmediaplayer
PRODUCT_PACKAGES += qcmediaplayer

# Misc
PRODUCT_PACKAGES += \
    com.android.future.usb.accessory \
    Torch

# Live wallpapers picker
PRODUCT_PACKAGES += LiveWallpapersPicker

# we have enough storage space to hold precise GC data
PRODUCT_TAGS += dalvik.gc.type-precise

# Common device properties
PRODUCT_PROPERTY_OVERRIDES += \
    rild.libpath=/system/lib/libril-qc-1.so \
    rild.libargs=-d/dev/smd0 \
    ro.ril.hsxpa=1 \
    ro.ril.gprsclass=10 \
    ro.telephony.call_ring.multiple=false \
    persist.ro.ril.sms_sync_sending=1 \
    ro.ril.def.agps.mode=2 \
    ro.ril.def.agps.feature=1

PRODUCT_PROPERTY_OVERRIDES += \
    wifi.supplicant_scan_interval=15 \
    ro.com.google.locationfeatures=1 \
    ro.com.google.clientidbase.ms=android-sonymobile \
    ro.product.locale.language=en \
    ro.product.locale.region=US \
    ro.use_data_netmgrd=true \
    ro.tethering.kb_disconnect=1

# Graphics
PRODUCT_PROPERTY_OVERRIDES += \
    debug.sf.hw=1 \
    debug.composition.type=dyn \
    persist.hwc.mdpcomp.enable=false \
    debug.mdpcomp.maxlayer=3 \
    debug.mdpcomp.logs=0

# The OpenGL ES API level that is natively supported by this device.
# This is a 16.16 fixed point number.
PRODUCT_PROPERTY_OVERRIDES += \
    ro.opengles.version=131072

# Low Power Audio
PRODUCT_PROPERTY_OVERRIDES += \
    lpa.decode=false \
    use.non-omx.mp3.decoder=false \
    use.non-omx.aac.decoder=false

# Disable gapless mode
PRODUCT_PROPERTY_OVERRIDES += \
    audio.gapless.playback.disable=true

# Set default USB interface
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.usb.config=mtp

# Increase speed for UMS transfer
PRODUCT_PROPERTY_OVERRIDES += \
    ro.vold.umsdirtyratio=20

# Enable repeatable keys in CWM
PRODUCT_PROPERTY_OVERRIDES += \
    ro.cwm.enable_key_repeat=true

# Prefer .tar backup format in CWM
PRODUCT_PROPERTY_OVERRIDES += \
    ro.cwm.prefer_tar=true

# For applications to determine if they should turn off specific memory-intensive
# features that work poorly on low-memory devices.
PRODUCT_PROPERTY_OVERRIDES += \
    ro.config.low_ram=true

# Reduce background apps limit to 16 on low-tier devices
PRODUCT_PROPERTY_OVERRIDES += \
    ro.sys.fw.bg_apps_limit=16

# Set max background services
PRODUCT_PROPERTY_OVERRIDES += \
    ro.config.max_starting_bg=8

# Disable strict mode
PRODUCT_PROPERTY_OVERRIDES += \
    persist.sys.strictmode.visual=0 \
    persist.sys.strictmode.disable=1

# proprietary side of the board
$(call inherit-product, vendor/semc/msm7x30-common/msm7x30-common-vendor.mk)
