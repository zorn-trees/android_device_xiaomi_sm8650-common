/*
 * Copyright (C) 2024 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "SensorNotifierUtils"

#include "SensorNotifierUtils.h"

#include <android-base/logging.h>

#include <cstdlib>
#include <cstring>
#include <unistd.h>

bool readBool(int fd) {
    char c;
    int rc;

    rc = lseek(fd, 0, SEEK_SET);
    if (rc) {
        LOG(ERROR) << "failed to seek fd, err: " << rc;
        return false;
    }

    rc = read(fd, &c, sizeof(char));
    if (rc != 1) {
        LOG(ERROR) << "failed to read bool from fd, err: " << rc;
        return false;
    }

    return c != '0';
}

disp_event_resp* parseDispEvent(int fd) {
    // mi_disp only hands out whole events: a read() buffer smaller than the
    // pending event returns 0 and leaves it queued, so read it in one go.
    char buf[1024];
    ssize_t size = read(fd, buf, sizeof(buf));
    if (size < static_cast<ssize_t>(sizeof(disp_event))) {
        LOG(ERROR) << "unexpected display event size: " << size;
        return nullptr;
    }

    disp_event* header = reinterpret_cast<disp_event*>(buf);
    if (header->length < sizeof(disp_event) || static_cast<ssize_t>(header->length) > size) {
        LOG(ERROR) << "invalid display event length: " << header->length;
        return nullptr;
    }

    struct disp_event_resp* response =
            reinterpret_cast<struct disp_event_resp*>(malloc(header->length));
    if (response == nullptr) {
        return nullptr;
    }
    memcpy(response, buf, header->length);

    return response;
}
