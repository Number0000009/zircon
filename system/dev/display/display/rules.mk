# Copyright 2018 The Fuchsia Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

LOCAL_DIR := $(GET_LOCAL_DIR)

MODULE := $(LOCAL_DIR)

MODULE_TYPE := driver

MODULE_SRCS := \
    $(LOCAL_DIR)/bind.c \
    $(LOCAL_DIR)/client.cpp \
    $(LOCAL_DIR)/controller.cpp \
    $(LOCAL_DIR)/fence.cpp \
    $(LOCAL_DIR)/image.cpp \

MODULE_STATIC_LIBS := \
    system/ulib/async.cpp \
    system/ulib/async \
    system/ulib/async-loop.cpp \
    system/ulib/async-loop \
    system/ulib/audio-proto-utils \
    system/ulib/ddk \
    system/ulib/ddktl \
    system/ulib/edid \
    system/ulib/hwreg \
    system/ulib/fbl \
    system/ulib/fidl \
    system/ulib/zx \
    system/ulib/zxcpp \

MODULE_FIDL_LIBS := system/fidl/fuchsia-hardware-display

MODULE_LIBS := \
	system/ulib/zircon \
	system/ulib/c \
	system/ulib/driver \
	system/ulib/async.default

MODULE_BANJO_LIBS := \
    system/banjo/ddk-protocol-display-controller \
    system/banjo/ddk-protocol-i2cimpl \

ifeq ($(call TOBOOL,$(ENABLE_DRIVER_TRACING)),true)
MODULE_STATIC_LIBS += system/ulib/trace.driver
endif
MODULE_HEADER_DEPS += system/ulib/trace system/ulib/trace-engine

include make/module.mk
