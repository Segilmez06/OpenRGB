/*---------------------------------------------------------*\
| HyperXPulsefireRaidController.cpp                         |
|                                                           |
|   Driver for HyperX Pulsefire Raid                        |
|                                                           |
|   Morgan Guimard (morg)                       06 Apr 2022 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cstring>
#include <thread>
#include "HyperXPulsefireRaidController.h"
#include "StringUtils.h"

using namespace std::chrono_literals;

HyperXPulsefireRaidController::HyperXPulsefireRaidController(hid_device* dev_handle, const hid_device_info& info, std::string dev_name)
{
    dev         = dev_handle;
    location    = info.path;
    name        = dev_name;

    memset(cached_profile, 0x00, sizeof(cached_profile));

    /*-----------------------------------------------------*\
    | Read existing hardware profile to preserve DPI stages |
    | and polling rate registers. If read fails, initialize |
    | safe factory defaults.                                |
    \*-----------------------------------------------------*/
    if(!ReadProfile(cached_profile, HYPERX_PULSEFIRE_RAID_SLOT_FLASH))
    {
        cached_profile[14] = 100; // Brightness 100%
        cached_profile[16] = 1;   // 1000 Hz polling rate

        // Default DPI stages: 400, 800, 1600, 3200 (scaled by / 50)
        unsigned short default_dpis[5] = { 8, 16, 32, 64, 0 };
        for(unsigned int i = 0; i < 5; i++)
        {
            cached_profile[17 + (i * 2)]     = (default_dpis[i] >> 8) & 0xFF;
            cached_profile[17 + (i * 2) + 1] = default_dpis[i] & 0xFF;

            cached_profile[29 + (i * 2)]     = (default_dpis[i] >> 8) & 0xFF;
            cached_profile[29 + (i * 2) + 1] = default_dpis[i] & 0xFF;

            cached_profile[42 + i]           = (i < 4) ? 1 : 0; // Bans
        }

        cached_profile[41] = 1; // Stage 2 active
        cached_profile[47] = HYPERX_PULSEFIRE_RAID_LED_MODE_STATIC;
        cached_profile[48] = HYPERX_PULSEFIRE_RAID_LED_MODE_STATIC;

        // Default static colors
        cached_profile[54] = 0x00; // Logo Red
        cached_profile[55] = 0xFF; // Logo Green
        cached_profile[56] = 0x00; // Logo Blue

        cached_profile[57] = 0x00; // Wheel Red
        cached_profile[58] = 0x00; // Wheel Green
        cached_profile[59] = 0xFF; // Wheel Blue

        // Dedicated DPI confirmation flash colors
        unsigned char default_dpi_colors[5][3] =
        {
            { 0xFF, 0x00, 0x00 }, // Stage 1: Red
            { 0xFF, 0xA5, 0x00 }, // Stage 2: Orange
            { 0x00, 0xFF, 0x00 }, // Stage 3: Green
            { 0x00, 0x00, 0xFF }, // Stage 4: Blue
            { 0x00, 0x00, 0x00 }  // Stage 5: Off
        };

        for(unsigned int i = 0; i < 5; i++)
        {
            cached_profile[97 + (i * 3)]     = default_dpi_colors[i][0];
            cached_profile[97 + (i * 3) + 1] = default_dpi_colors[i][1];
            cached_profile[97 + (i * 3) + 2] = default_dpi_colors[i][2];
        }

        cached_profile[112] = 1;  // 1 frame
        cached_profile[113] = 50; // 50 ms
    }
}

HyperXPulsefireRaidController::~HyperXPulsefireRaidController()
{
    hid_close(dev);
}

std::string HyperXPulsefireRaidController::GetDeviceLocation()
{
    return("HID: " + location);
}

std::string HyperXPulsefireRaidController::GetNameString()
{
    return(name);
}

std::string HyperXPulsefireRaidController::GetSerialString()
{
    wchar_t serial_string[128];
    int ret = hid_get_serial_number_string(dev, serial_string, 128);

    if(ret != 0)
    {
        return("");
    }

    return(StringUtils::wstring_to_string(serial_string));
}

void HyperXPulsefireRaidController::SendColors(std::vector<RGBColor> colors)
{
    unsigned char usb_buf[HYPERX_PULSFIRE_RAID_PACKET_DATA_LENGTH];
    memset(usb_buf, 0x00, sizeof(usb_buf));

    usb_buf[0] = HYPERX_PULSFIRE_RAID_REPORT_ID;
    usb_buf[1] = HYPERX_PULSEFIRE_RAID_DIRECT_MODE_START_PACKET;

    for(unsigned int i = 0; i < 2 && i < colors.size(); i++)
    {
        usb_buf[3 * i + 2] = RGBGetRValue(colors[i]);
        usb_buf[3 * i + 3] = RGBGetGValue(colors[i]);
        usb_buf[3 * i + 4] = RGBGetBValue(colors[i]);
    }

    usb_buf[8] = HYPERX_PULSEFIRE_RAID_DIRECT_MODE_END_PACKET;

    Send(usb_buf);
}

void HyperXPulsefireRaidController::SetBrightness(unsigned char brightness)
{
    ApplyProfile(brightness, HYPERX_PULSEFIRE_RAID_SLOT_RUNTIME);
}

bool HyperXPulsefireRaidController::ReadProfile(unsigned char* buffer_256, unsigned char slot)
{
    unsigned char req[HYPERX_PULSFIRE_RAID_PACKET_DATA_LENGTH];
    memset(req, 0x00, sizeof(req));

    req[0] = HYPERX_PULSFIRE_RAID_REPORT_ID;
    req[1] = HYPERX_PULSEFIRE_RAID_CMD_GET_PROFILE;
    req[2] = slot;

    Send(req);
    std::this_thread::sleep_for(100ms);

    unsigned char resp[HYPERX_PULSFIRE_RAID_PACKET_DATA_LENGTH];
    memset(resp, 0x00, sizeof(resp));
    resp[0] = HYPERX_PULSFIRE_RAID_REPORT_ID;

    int ret = hid_get_feature_report(dev, resp, sizeof(resp));
    if(ret < 0 || resp[1] != HYPERX_PULSEFIRE_RAID_CMD_GET_PROFILE)
    {
        std::this_thread::sleep_for(50ms);
        ret = hid_get_feature_report(dev, resp, sizeof(resp));
        if(ret < 0 || resp[1] != HYPERX_PULSEFIRE_RAID_CMD_GET_PROFILE)
        {
            return(false);
        }
    }

    memcpy(buffer_256, &resp[8], 256);
    return(true);
}

void HyperXPulsefireRaidController::SaveProfile(const unsigned char* buffer_256, unsigned char brightness, unsigned char slot)
{
    unsigned char packet[HYPERX_PULSFIRE_RAID_PACKET_DATA_LENGTH];
    memset(packet, 0x00, sizeof(packet));

    packet[0] = HYPERX_PULSFIRE_RAID_REPORT_ID;
    packet[1] = HYPERX_PULSEFIRE_RAID_CMD_SET_PROFILE;
    packet[2] = slot;
    packet[3] = brightness;
    memcpy(&packet[8], buffer_256, 256);

    Send(packet);
    std::this_thread::sleep_for(50ms);

    ApplyProfile(brightness, slot);
}

void HyperXPulsefireRaidController::ApplyProfile(unsigned char brightness, unsigned char slot)
{
    unsigned char packet[HYPERX_PULSFIRE_RAID_PACKET_DATA_LENGTH];
    memset(packet, 0x00, sizeof(packet));

    packet[0] = HYPERX_PULSFIRE_RAID_REPORT_ID;
    packet[1] = HYPERX_PULSEFIRE_RAID_CMD_APPLY_PROFILE;
    packet[2] = slot;
    packet[3] = brightness;

    Send(packet);
    std::this_thread::sleep_for(30ms);
}

void HyperXPulsefireRaidController::UploadAnimationFrames(const unsigned char* frame_data, unsigned int frame_count, unsigned char slot)
{
    if(frame_count > 128)
    {
        frame_count = 128;
    }

    unsigned int total_len = frame_count * 6;
    unsigned int offset    = 0;

    for(unsigned int chunk_idx = 0; chunk_idx < 3; chunk_idx++)
    {
        unsigned int chunk_len = total_len - offset;
        if(chunk_len > 256)
        {
            chunk_len = 256;
        }

        if(chunk_len == 0 && chunk_idx > 0)
        {
            break;
        }

        unsigned char packet[HYPERX_PULSFIRE_RAID_PACKET_DATA_LENGTH];
        memset(packet, 0x00, sizeof(packet));

        packet[0] = HYPERX_PULSFIRE_RAID_REPORT_ID;
        packet[1] = HYPERX_PULSEFIRE_RAID_CMD_UPLOAD_FRAMES;
        packet[2] = slot;
        packet[4] = chunk_idx;

        if(chunk_len > 0)
        {
            memcpy(&packet[8], &frame_data[offset], chunk_len);
            offset += chunk_len;
        }

        Send(packet);
        std::this_thread::sleep_for(50ms);
    }
}

unsigned char* HyperXPulsefireRaidController::GetCachedProfile()
{
    return(cached_profile);
}

void HyperXPulsefireRaidController::Send(unsigned char* packet)
{
    hid_send_feature_report(dev, packet, HYPERX_PULSFIRE_RAID_PACKET_DATA_LENGTH);
    std::this_thread::sleep_for(10ms);
}
