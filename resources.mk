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

COMMON_PATH := device/semc/msm7x30-common

# Offline charging animation
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/animations/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/charging_animation_01.png:system/semc/chargemon/data/charging_animation_01.png \
    $(COMMON_PATH)/animations/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/charging_animation_02.png:system/semc/chargemon/data/charging_animation_02.png \
    $(COMMON_PATH)/animations/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/charging_animation_03.png:system/semc/chargemon/data/charging_animation_03.png \
    $(COMMON_PATH)/animations/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/charging_animation_04.png:system/semc/chargemon/data/charging_animation_04.png \
    $(COMMON_PATH)/animations/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/charging_animation_05.png:system/semc/chargemon/data/charging_animation_05.png \
    $(COMMON_PATH)/animations/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/charging_animation_06.png:system/semc/chargemon/data/charging_animation_06.png \
    $(COMMON_PATH)/animations/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/charging_animation_07.png:system/semc/chargemon/data/charging_animation_07.png

# Animated boot logo
PRODUCT_COPY_FILES += \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/1.rle:root/bootlogo/1.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/2.rle:root/bootlogo/2.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/3.rle:root/bootlogo/3.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/4.rle:root/bootlogo/4.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/5.rle:root/bootlogo/5.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/6.rle:root/bootlogo/6.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/7.rle:root/bootlogo/7.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/8.rle:root/bootlogo/8.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/9.rle:root/bootlogo/9.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/10.rle:root/bootlogo/10.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/11.rle:root/bootlogo/11.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/12.rle:root/bootlogo/12.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/13.rle:root/bootlogo/13.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/14.rle:root/bootlogo/14.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/15.rle:root/bootlogo/15.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/16.rle:root/bootlogo/16.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/17.rle:root/bootlogo/17.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/18.rle:root/bootlogo/18.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/19.rle:root/bootlogo/19.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/20.rle:root/bootlogo/20.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/21.rle:root/bootlogo/21.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/22.rle:root/bootlogo/22.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/23.rle:root/bootlogo/23.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/24.rle:root/bootlogo/24.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/25.rle:root/bootlogo/25.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/26.rle:root/bootlogo/26.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/27.rle:root/bootlogo/27.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/28.rle:root/bootlogo/28.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/29.rle:root/bootlogo/29.rle \
    $(COMMON_PATH)/bootlogo/$(TARGET_SCREEN_WIDTH)x$(TARGET_SCREEN_HEIGHT)/30.rle:root/bootlogo/30.rle
