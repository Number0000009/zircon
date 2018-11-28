// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ddk/binding.h>
#include <ddk/driver.h>
#include <ddktl/device.h>
#include <ddktl/protocol/test.h>
#include <fbl/algorithm.h>
#include <fbl/string_piece.h>
#include <fbl/unique_ptr.h>
#include <fuchsia/device/test/c/fidl.h>
#include <lib/zx/channel.h>
#include <lib/zx/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zircon/device/test.h>

namespace {

class TestDevice;
using TestDeviceType = ddk::Device<TestDevice, ddk::Ioctlable, ddk::Messageable>;

class TestDevice : public TestDeviceType,
                   public ddk::TestProtocol<TestDevice> {
public:
    TestDevice(zx_device_t* parent) : TestDeviceType(parent) { }

    // Methods required by the ddk mixins
    zx_status_t DdkIoctl(uint32_t op, const void* in, size_t inlen, void* out,
                         size_t outlen, size_t* out_actual);
    zx_status_t DdkMessage(fidl_msg_t* msg, fidl_txn_t* txn);
    void DdkRelease();

    // Methods required by the TestProtocol mixin
    void TestSetOutputSocket(zx_handle_t handle);
    void TestGetOutputSocket(zx_handle_t* output_handle);
    void TestSetTestFunc(const test_func_t* func);
    zx_status_t TestRunTests(test_report_t* out_report);
    void TestDestroy();
private:
    zx::socket output_;
    test_func_t test_func_;
};

class TestRootDevice;
using TestRootDeviceType = ddk::Device<TestRootDevice, ddk::Ioctlable, ddk::Messageable>;

class TestRootDevice : public TestRootDeviceType {
public:
    TestRootDevice(zx_device_t* parent) : TestRootDeviceType(parent) { }

    zx_status_t Bind() {
        return DdkAdd("test");
    }

    // Methods required by the ddk mixins
    zx_status_t DdkIoctl(uint32_t op, const void* in, size_t inlen, void* out,
                         size_t outlen, size_t* out_actual);
    zx_status_t DdkMessage(fidl_msg_t* msg, fidl_txn_t* txn);
    void DdkRelease() { ZX_ASSERT_MSG(false, "TestRootDevice::DdkRelease() not supported\n"); }

    static zx_status_t FidlCreateDevice(void* ctx, const char* name_data, size_t name_len,
                                        fidl_txn_t* txn);
private:
    // Create a new child device with this |name|
    zx_status_t CreateDevice(const fbl::StringPiece& name,
                             char* path_out, size_t path_size, size_t* path_actual);
};

void TestDevice::TestSetOutputSocket(zx_handle_t handle) {
    output_.reset(handle);
}

void TestDevice::TestGetOutputSocket(zx_handle_t* output_handle) {
    *output_handle = output_.get();
}

void TestDevice::TestSetTestFunc(const test_func_t* func) {
    test_func_ = *func;
}

zx_status_t TestDevice::TestRunTests(test_report_t* report) {
    if (test_func_.callback == NULL) {
        return ZX_ERR_NOT_SUPPORTED;
    }
    return test_func_.callback(test_func_.ctx, report);
}

void TestDevice::TestDestroy() {
    DdkRemove();
}

static zx_status_t fidl_SetOutputSocket(void* ctx, zx_handle_t raw_socket) {
    zx::socket socket(raw_socket);
    auto dev = static_cast<TestDevice*>(ctx);
    dev->TestSetOutputSocket(socket.release());
    return ZX_OK;
}

static zx_status_t fidl_RunTests(void* ctx, fidl_txn_t* txn) {
    auto dev = static_cast<TestDevice*>(ctx);
    test_report_t report = {};
    fuchsia_device_test_TestReport fidl_report = {};

    zx_status_t status = dev->TestRunTests(&report);
    if (status == ZX_OK) {
        fidl_report.test_count = report.n_tests;
        fidl_report.success_count = report.n_success;
        fidl_report.failure_count = report.n_failed;
    }
    return fuchsia_device_test_DeviceRunTests_reply(txn, status, &fidl_report);
}

static zx_status_t fidl_Destroy(void* ctx) {
    auto dev = static_cast<TestDevice*>(ctx);
    dev->TestDestroy();
    return ZX_OK;
}

zx_status_t TestDevice::DdkMessage(fidl_msg_t* msg, fidl_txn_t* txn) {
    static const fuchsia_device_test_Device_ops_t kOps = {
        .SetOutputSocket = fidl_SetOutputSocket,
        .RunTests = fidl_RunTests,
        .Destroy = fidl_Destroy,
    };
    return fuchsia_device_test_Device_dispatch(this, txn, msg, &kOps);
}

zx_status_t TestDevice::DdkIoctl(uint32_t op, const void* in, size_t inlen, void* out,
                                 size_t outlen, size_t* out_actual) {
    switch (op) {
    case IOCTL_TEST_SET_OUTPUT_SOCKET:
        if (inlen != sizeof(zx_handle_t)) {
            return ZX_ERR_INVALID_ARGS;
        }
        TestSetOutputSocket(*(zx_handle_t*)in);
        return ZX_OK;

    case IOCTL_TEST_RUN_TESTS: {
        if (outlen != sizeof(test_report_t)) {
            return ZX_ERR_BUFFER_TOO_SMALL;
        }
        *out_actual = sizeof(test_report_t);
        return TestRunTests(static_cast<test_report_t*>(out));
    }

    case IOCTL_TEST_DESTROY_DEVICE:
        TestDestroy();
        return 0;

    default:
        return ZX_ERR_NOT_SUPPORTED;
    }
}

void TestDevice::DdkRelease() {
    delete this;
}

zx_status_t TestRootDevice::CreateDevice(const fbl::StringPiece& name,
                                         char* path_out, size_t path_size, size_t* path_actual) {
    static_assert(fuchsia_device_test_MAX_DEVICE_NAME_LEN == ZX_DEVICE_NAME_MAX);

    char devname[ZX_DEVICE_NAME_MAX + 1] = {};
    if (name.size() > 0) {
        memcpy(devname, name.data(), fbl::min(sizeof(devname) - 1, name.size()));
    } else {
        strncpy(devname, "testdev", sizeof(devname) - 1);
    }
    devname[sizeof(devname) - 1] = '\0';
    // truncate trailing ".so"
    if (!strcmp(devname + strlen(devname) - 3, ".so")) {
        devname[strlen(devname) - 3] = 0;
    }

    if (path_size < strlen(devname) + sizeof(fuchsia_device_test_CONTROL_DEVICE) + 1) {
        return ZX_ERR_BUFFER_TOO_SMALL;
    }

    auto device = fbl::make_unique<TestDevice>(zxdev());
    zx_status_t status = device->DdkAdd(devname);
    if (status != ZX_OK) {
        return status;
    }
    // devmgr now owns this
    __UNUSED auto ptr = device.release();

    *path_actual = snprintf(path_out, path_size ,"%s/%s", fuchsia_device_test_CONTROL_DEVICE,
                            devname);
    return ZX_OK;
}

zx_status_t TestRootDevice::DdkIoctl(uint32_t op, const void* in, size_t inlen,
                                     void* out, size_t outlen, size_t* out_actual) {
    if (op != IOCTL_TEST_CREATE_DEVICE) {
        return ZX_ERR_NOT_SUPPORTED;
    }

    zx_status_t status = CreateDevice(fbl::StringPiece(static_cast<const char*>(in)),
                                      static_cast<char*>(out), outlen, out_actual);
    if (status != ZX_OK) {
        return status;
    }
    if (*out_actual == outlen) {
        // Force null-termination if we filled the buffer.
        static_cast<char*>(out)[outlen - 1] = 0;
    } else {
        // Account for the trailing null byte which is reported in the ioctl return
        *out_actual += 1;
    }
    return ZX_OK;
}

zx_status_t TestRootDevice::FidlCreateDevice(void* ctx, const char* name_data, size_t name_len,
                                              fidl_txn_t* txn) {
    auto root = static_cast<TestRootDevice*>(ctx);

    char path[fuchsia_device_test_MAX_DEVICE_PATH_LEN];
    size_t path_size = 0;
    zx_status_t status = root->CreateDevice(fbl::StringPiece(name_data, name_len),
                                            path, sizeof(path), &path_size);
    return fuchsia_device_test_RootDeviceCreateDevice_reply(txn, status, path, path_size);
}

zx_status_t TestRootDevice::DdkMessage(fidl_msg_t* msg, fidl_txn_t* txn) {
    static const fuchsia_device_test_RootDevice_ops_t kOps = {
        .CreateDevice = TestRootDevice::FidlCreateDevice,
    };
    return fuchsia_device_test_RootDevice_dispatch(this, txn, msg, &kOps);
}

zx_status_t TestDriverBind(void* ctx, zx_device_t* dev) {
    auto root = fbl::make_unique<TestRootDevice>(dev);
    zx_status_t status = root->Bind();
    if (status != ZX_OK) {
        return status;
    }
    // devmgr now owns root
    __UNUSED auto ptr = root.release();
    return ZX_OK;
}

const zx_driver_ops_t kTestDriverOps = []() {
    zx_driver_ops_t driver;
    driver.version = DRIVER_OPS_VERSION;
    driver.bind = TestDriverBind;
    return driver;
}();

} // namespace

ZIRCON_DRIVER_BEGIN(test, kTestDriverOps, "zircon", "0.1", 1)
    BI_MATCH_IF(EQ, BIND_PROTOCOL, ZX_PROTOCOL_TEST_PARENT),
ZIRCON_DRIVER_END(test)
