LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := EnemySaga
LOCAL_SRC_FILES := main.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/imgui
LOCAL_LDLIBS := -llog -landroid -lEGL -lGLESv2

include $(BUILD_SHARED_LIBRARY)
