# This is the Android.mk file for smart controller programm
# Authot : VIA (Networking Division)
# Date : 06/1/2015 (DD/MM/YY)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LIBUSB_DIR := external/libusb-1.0.9

LOCAL_MODULE := smc
LOCAL_MODULE_TAGS := optional eng
LOCAL_SRC_FILES := smc.c

LOCAL_C_INCLUDES += $(LOCAL_PATH) \
	$(LIBUSB_DIR) \
	$(LIBUSB_DIR)/libusb

#LOCAL_CFLAGS +=
#LOCAL_LDLIBS +=

LOCAL_SHARED_LIBRARIES := libc libusb
#LOCAL_STATIC_LIBRARIES := 
include $(BUILD_EXECUTABLE)

