# Copyright 2017 The Fuchsia Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_TYPE := driver

MODULE_SRCS += \
    $(LOCAL_DIR)/gauss.c \
    $(LOCAL_DIR)/gauss-audio.c \
    $(LOCAL_DIR)/gauss-clk.c \
    $(LOCAL_DIR)/gauss-sysmem.c \
    $(LOCAL_DIR)/gauss-gpio.c \
    $(LOCAL_DIR)/gauss-i2c.c \
    $(LOCAL_DIR)/gauss-pcie.c \
    $(LOCAL_DIR)/gauss-usb.c \
    $(LOCAL_DIR)/gauss-rawnand.c \

MODULE_HEADER_DEPS := \
    system/dev/pci/designware

MODULE_STATIC_LIBS := \
    system/dev/lib/amlogic \
    system/ulib/ddk \
    system/ulib/sync

MODULE_LIBS := \
    system/ulib/driver \
    system/ulib/c \
    system/ulib/zircon

MODULE_BANJO_LIBS := \
    system/banjo/ddk-protocol-gpio \
    system/banjo/ddk-protocol-gpioimpl \
    system/banjo/ddk-protocol-iommu \
    system/banjo/ddk-protocol-platform-bus \
    system/banjo/ddk-protocol-platform-device \

include make/module.mk

MODULE := $(LOCAL_DIR).i2c-test

MODULE_NAME := gauss-i2c-test

MODULE_TYPE := driver

MODULE_SRCS += \
    $(LOCAL_DIR)/gauss-i2c-test.c \

MODULE_STATIC_LIBS := system/ulib/ddk system/ulib/sync

MODULE_LIBS := \
    system/ulib/driver \
    system/ulib/c \
    system/ulib/zircon

MODULE_BANJO_LIBS := \
    system/banjo/ddk-protocol-i2c \
    system/banjo/ddk-protocol-platform-device \

include make/module.mk

ifeq (PLEASE_DISCUSS_WITH_SWETLAND,)
MODULE := $(LOCAL_DIR).led

MODULE_NAME := gauss-led

MODULE_TYPE := driver

MODULE_SRCS += \
    $(LOCAL_DIR)/gauss-led.c

MODULE_STATIC_LIBS := system/ulib/ddk system/ulib/sync

MODULE_LIBS := system/ulib/driver system/ulib/c system/ulib/zircon

include make/module.mk
endif
