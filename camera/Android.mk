LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    CameraWrapper.cpp

LOCAL_SHARED_LIBRARIES := \
    libhardware liblog libcamera_client libutils

ifeq ($(TARGET_DEVICE),urushi)
    LOCAL_CFLAGS += -DUSES_AS3676_TORCH
endif

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE := camera.semc

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
