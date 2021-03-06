// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <ddk/device.h>
#include <ddk/protocol/gpioimpl.h>
#include <ddk/protocol/iommu.h>
#include <ddk/protocol/platform/bus.h>
#include <soc/aml-s905d2/s905d2-gpio.h>

// BTI IDs for our devices
enum {
    BTI_BOARD,
    BTI_USB_XHCI,
    BTI_DISPLAY,
    BTI_MALI,
    BTI_VIDEO,
    BTI_AML_RAW_NAND,
    BTI_SDIO,
    BTI_CANVAS,
    BTI_THERMAL,
    BTI_AUDIO_IN,
    BTI_AUDIO_OUT,
    BTI_TEE,
    BTI_SYSMEM,
};

// MAC address metadata indices
enum {
    MACADDR_WIFI = 0,
    MACADDR_BLUETOOTH = 1,
};

typedef struct {
    zx_device_t* parent;
    pbus_protocol_t pbus;
    gpio_impl_protocol_t gpio;
    iommu_protocol_t iommu;
} aml_bus_t;

// astro-sysmem.c
zx_status_t astro_sysmem_init(aml_bus_t* bus);

// astro-gpio.c
zx_status_t aml_gpio_init(aml_bus_t* bus);

// astro-i2c.c
zx_status_t aml_i2c_init(aml_bus_t* bus);

// astro-bluetooth.c
zx_status_t aml_bluetooth_init(aml_bus_t* bus);

// astro-usb.c
zx_status_t aml_usb_init(aml_bus_t* bus);

// astro-display.c
zx_status_t aml_display_init(aml_bus_t* bus);

// astro-backlight.c
zx_status_t astro_backlight_init(aml_bus_t* bus);

// These should match the mmio table defined in astro-i2c.c
enum {
    ASTRO_I2C_A0_0,
    ASTRO_I2C_2,
    ASTRO_I2C_3,
};

// Astro Board Revs
enum {
    BOARD_REV_P1            = 0,
    BOARD_REV_P2            = 1,
    BOARD_REV_EVT_1         = 2,
    BOARD_REV_EVT_2         = 3,

    MAX_SUPPORTED_REV, // This must be last entry
};

// Astro GPIO Pins used for board rev detection
#define GPIO_HW_ID0             (S905D2_GPIOZ(7))
#define GPIO_HW_ID1             (S905D2_GPIOZ(8))
#define GPIO_HW_ID2             (S905D2_GPIOZ(3))

/* Astro I2C Devices */
#define I2C_BACKLIGHT_ADDR    (0x2C)
#define I2C_AMBIENTLIGHT_ADDR (0x39)
// astro-touch.c
zx_status_t astro_touch_init(aml_bus_t* bus);
// aml-raw_nand.c
zx_status_t aml_raw_nand_init(aml_bus_t* bus);
// astro-sdio.c
zx_status_t aml_sdio_init(aml_bus_t* bus);
// astro-canvas.c
zx_status_t aml_canvas_init(aml_bus_t* bus);
// astro-light.c
zx_status_t ams_light_init(aml_bus_t* bus);
// astro-thermal.c
zx_status_t aml_thermal_init(aml_bus_t* bus);
// astro-video.c
zx_status_t aml_video_init(aml_bus_t* bus);
// astro-clk.c
zx_status_t aml_clk_init(aml_bus_t* bus);
// astro-audio.c
zx_status_t astro_tdm_init(aml_bus_t* bus);
// astro-tee.c
zx_status_t astro_tee_init(aml_bus_t* bus);
// astro-buttons.c
zx_status_t astro_buttons_init(aml_bus_t* bus);
