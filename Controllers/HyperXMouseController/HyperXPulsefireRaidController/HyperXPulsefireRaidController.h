/*---------------------------------------------------------*\
| HyperXPulsefireRaidController.h                           |
|                                                           |
|   Driver for HyperX Pulsefire Raid                        |
|                                                           |
|   Morgan Guimard (morg)                       06 Apr 2022 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include <hidapi.h>
#include "RGBController.h"

#define HYPERX_PULSFIRE_RAID_PACKET_DATA_LENGTH         264
#define HYPERX_PULSFIRE_RAID_REPORT_ID                  0x07
#define HYPERX_PULSFIRE_RAID_LEDS_COUNT                 2

#define HYPERX_PULSEFIRE_RAID_CMD_SET_PROFILE           0x01
#define HYPERX_PULSEFIRE_RAID_CMD_APPLY_PROFILE         0x03
#define HYPERX_PULSEFIRE_RAID_CMD_SET_DIRECT_LED        0x0A
#define HYPERX_PULSEFIRE_RAID_CMD_UPLOAD_FRAMES         0x18
#define HYPERX_PULSEFIRE_RAID_CMD_GET_PROFILE           0x81

#define HYPERX_PULSEFIRE_RAID_SLOT_FLASH                0x01
#define HYPERX_PULSEFIRE_RAID_SLOT_RUNTIME              0x04

#define HYPERX_PULSEFIRE_RAID_LED_MODE_STATIC           0x01
#define HYPERX_PULSEFIRE_RAID_LED_MODE_TRIGGER          0x03
#define HYPERX_PULSEFIRE_RAID_LED_MODE_SYNC             0x04
#define HYPERX_PULSEFIRE_RAID_LED_MODE_CYCLE            0x05

#define HYPERX_PULSEFIRE_RAID_DIRECT_MODE_START_PACKET  0x0A
#define HYPERX_PULSEFIRE_RAID_DIRECT_MODE_END_PACKET    0xA0

enum
{
    HYPERX_PULSFIRE_RAID_BRIGHTNESS_MIN = 0x00,
    HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX = 0x64
};

class HyperXPulsefireRaidController
{
public:
    HyperXPulsefireRaidController(hid_device* dev_handle, const hid_device_info& info, std::string dev_name);
    ~HyperXPulsefireRaidController();

    std::string     GetNameString();
    std::string     GetSerialString();
    std::string     GetDeviceLocation();

    void            SendColors(std::vector<RGBColor> colors);
    void            SetBrightness(unsigned char brightness);

    bool            ReadProfile(unsigned char* buffer_256, unsigned char slot = HYPERX_PULSEFIRE_RAID_SLOT_FLASH);
    void            SaveProfile(const unsigned char* buffer_256, unsigned char brightness, unsigned char slot = HYPERX_PULSEFIRE_RAID_SLOT_FLASH);
    void            ApplyProfile(unsigned char brightness, unsigned char slot = HYPERX_PULSEFIRE_RAID_SLOT_FLASH);
    void            UploadAnimationFrames(const unsigned char* frame_data, unsigned int frame_count, unsigned char slot = HYPERX_PULSEFIRE_RAID_SLOT_FLASH);

    unsigned char*  GetCachedProfile();

protected:
    hid_device*     dev;

private:
    std::string     location;
    std::string     name;
    unsigned char   cached_profile[256];

    void            Send(unsigned char* packet);
};
