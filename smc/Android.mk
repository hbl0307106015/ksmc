# This is the Android.mk file for smart controller programm
# Authot : VIA (Networking Division)
# Date : 06/1/2015 (DD/MM/YY)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := smc
LOCAL_MODULE_TAGS := optional eng
LOCAL_SRC_FILES := main.c \
	circularQueue.c log.c smcNetwork.c smcPeerUtility.c \
	smcProtocol.c smcQueue.c threadAPP.c threadKNX.c \
	threadZigBee.c

LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_CFLAGS += -I$(LOCAL_C_INCLUDES)
LOCAL_LDLIBS += -lpthread

LOCAL_SHARED_LIBRARIES := libc 
#LOCAL_STATIC_LIBRARIES :=
include $(BUILD_EXECUTABLE)
