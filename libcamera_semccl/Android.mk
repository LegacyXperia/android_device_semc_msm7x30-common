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

LOCAL_PATH := $(call my-dir)/../../../../frameworks/av/camera

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
	Camera.cpp \
	CameraMetadata.cpp \
	CaptureResult.cpp \
	CameraParameters2.cpp \
	ICamera.cpp \
	ICameraClient.cpp \
	ICameraService.cpp \
	ICameraServiceListener.cpp \
	ICameraServiceProxy.cpp \
	ICameraRecordingProxy.cpp \
	ICameraRecordingProxyListener.cpp \
	camera2/ICameraDeviceUser.cpp \
	camera2/ICameraDeviceCallbacks.cpp \
	camera2/CaptureRequest.cpp \
	camera2/OutputConfiguration.cpp \
	CameraBase.cpp \
	CameraUtils.cpp \
	VendorTagDescriptor.cpp

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libsemcu \
	liblog \
	libbinder \
	libhardware \
	libui \
	libgui \
	libcamera_metadata

LOCAL_C_INCLUDES += \
	system/media/camera/include \
	system/media/private/camera/include

LOCAL_WHOLE_STATIC_LIBRARIES += libcamera_parameters

LOCAL_MODULE:= libcamera_semccl

include $(BUILD_SHARED_LIBRARY)
