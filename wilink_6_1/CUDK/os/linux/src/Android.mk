LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

STATIC_LIB ?= y
DEBUG ?= y
BUILD_SUPPL ?= n
WPA_ENTERPRISE ?= y

ifeq ($(DEBUG),y)
  DEBUGFLAGS = -O2 -g -DDEBUG -DTI_DBG -fno-builtin   
else
  DEBUGFLAGS = -O2
endif

WILINK_ROOT = ../../../..
CUDK_ROOT = $(WILINK_ROOT)/CUDK
TI_SUPP_LIB_DIR = $(WILINK_ROOT)/../../../../external/wpa_supplicant

DK_DEFINES = 
ifeq ($(WPA_ENTERPRISE), y)
        DK_DEFINES += -D WPA_ENTERPRISE
endif

ifeq ($(BUILD_SUPPL), y)
  DK_DEFINES += -D WPA_SUPPLICANT -D CONFIG_CTRL_IFACE -D CONFIG_CTRL_IFACE_UNIX
  -include external/wpa_supplicant/.config
  ifeq ($(CONFIG_EAP_WSC), y)
    DK_DEFINES += -DCONFIG_EAP_WSC
  endif
endif

LOCAL_CFLAGS += \
	-Wall -Wstrict-prototypes $(DEBUGFLAGS) -D__LINUX__ $(DK_DEFINES) -D__BYTE_ORDER_LITTLE_ENDIAN -fno-common #-pipe

LOCAL_SRC_FILES:= \
	cu_wext.c \
	ipc_sta.c \
	ipc_event.c \
	ipc_wpa.c \
	os_trans.c \
	ParsEvent.c \
	osapi.c

ifeq ($(BUILD_SUPPL), y)
LOCAL_SRC_FILES += \
	$(TI_SUPP_LIB_DIR)/wpa_ctrl.c
endif

LOCAL_C_INCLUDES := \
        $(LOCAL_PATH)/../inc \
        $(LOCAL_PATH)/../../common/inc \
        $(LOCAL_PATH)/$(WILINK_ROOT)/stad/Export_Inc \
        $(LOCAL_PATH)/$(WILINK_ROOT)/stad/src/Sta_Management \
        $(LOCAL_PATH)/$(WILINK_ROOT)/stad/src/Application \
        $(LOCAL_PATH)/$(WILINK_ROOT)/utils \
        $(LOCAL_PATH)/$(WILINK_ROOT)/Txn \
        $(LOCAL_PATH)/$(WILINK_ROOT)/TWD/TWDriver \
	$(LOCAL_PATH)/$(WILINK_ROOT)/TWD/FirmwareApi \
	$(LOCAL_PATH)/$(WILINK_ROOT)/TWD/FW_Transfer/Export_Inc \
	$(LOCAL_PATH)/$(WILINK_ROOT)/TWD/TwIf \
        $(LOCAL_PATH)/$(WILINK_ROOT)/platforms/os/linux/inc \
        $(LOCAL_PATH)/$(WILINK_ROOT)/platforms/os/common/inc \
        $(LOCAL_PATH)/$(WILINK_ROOT)/TWD/FirmwareApi \
        external/wpa_supplicant \
        $(LOCAL_PATH)/$(CUDK_ROOT)/configurationutility/inc

LOCAL_MODULE := libtiOsLib
LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)

