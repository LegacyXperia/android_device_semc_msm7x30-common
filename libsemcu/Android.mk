# Copyright (C) 2015 The CyanogenMod Project
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
LIBUTILS_PATH := $(LOCAL_PATH)/../../../../system/core/libutils

originalSources := \
	$(LIBUTILS_PATH)/BasicHashtable.cpp \
	$(LIBUTILS_PATH)/CallStack.cpp \
	$(LIBUTILS_PATH)/FileMap.cpp \
	$(LIBUTILS_PATH)/JenkinsHash.cpp \
	$(LIBUTILS_PATH)/LinearTransform.cpp \
	$(LIBUTILS_PATH)/Log.cpp \
	$(LIBUTILS_PATH)/NativeHandle.cpp \
	$(LIBUTILS_PATH)/Printer.cpp \
	$(LIBUTILS_PATH)/ProcessCallStack.cpp \
	$(LIBUTILS_PATH)/PropertyMap.cpp \
	$(LIBUTILS_PATH)/SharedBuffer.cpp \
	$(LIBUTILS_PATH)/Static.cpp \
	$(LIBUTILS_PATH)/StopWatch.cpp \
	$(LIBUTILS_PATH)/String8.cpp \
	$(LIBUTILS_PATH)/String16.cpp \
	$(LIBUTILS_PATH)/SystemClock.cpp \
	$(LIBUTILS_PATH)/Threads.cpp \
	$(LIBUTILS_PATH)/Timers.cpp \
	$(LIBUTILS_PATH)/Tokenizer.cpp \
	$(LIBUTILS_PATH)/Unicode.cpp \
	$(LIBUTILS_PATH)/misc.cpp

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	$(originalSources) \
	RefBase.cpp \
	VectorImpl.cpp

LOCAL_MODULE := libsemcu
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES += external/safe-iop/include
LOCAL_SHARED_LIBRARIES := libbacktrace libcutils libdl liblog

include $(BUILD_SHARED_LIBRARY)
