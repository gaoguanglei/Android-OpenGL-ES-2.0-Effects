# Copyright (C) 2010 The Android Open Source Project
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
#
LOCAL_PATH := $(call my-dir)

rendermonkey := $(LOCAL_PATH)/../../../

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
			$(rendermonkey)/common/ \
			$(rendermonkey)/platform/android/
LOCAL_MODULE    := fire_flame
LOCAL_SRC_FILES := $(rendermonkey)/fire_flame/main.cpp \
		 	$(rendermonkey)/common/3ds.cpp \
			$(rendermonkey)/common/load_tga.cpp \
			$(rendermonkey)/common/load_dds.cpp \
			$(rendermonkey)/common/shader_loader.cpp \
			$(rendermonkey)/common/shader_program.cpp \
			$(rendermonkey)/common/math_lib.cpp \
			$(rendermonkey)/common/shape.cpp \
			$(rendermonkey)/platform/android/android_main.cpp
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2
LOCAL_STATIC_LIBRARIES := android_native_app_glue
LOCAL_CFLAGS := -DANDROID_OPENGL

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
