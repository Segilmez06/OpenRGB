/*---------------------------------------------------------*\
| RGBController_HyperXPulsefireRaid.cpp                     |
|                                                           |
|   RGBController for HyperX Pulsefire Raid                 |
|                                                           |
|   Morgan Guimard (morg)                       06 Apr 2022 |
|                                                           |
|   This file is part of the OpenRGB project                |
|   SPDX-License-Identifier: GPL-2.0-or-later               |
\*---------------------------------------------------------*/

#include <cmath>
#include <cstring>
#include "RGBController_HyperXPulsefireRaid.h"

/**------------------------------------------------------------------*\
    @name HyperX Pulsefire Raid
    @category Mouse
    @type USB
    @save :white_check_mark:
    @direct :white_check_mark:
    @effects :white_check_mark:
    @detectors DetectHyperXPulsefireRaidControllers
    @comment Enhanced Autonomous Driver with On-board Persistence
\*-------------------------------------------------------------------*/

using namespace std::chrono_literals;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static RGBColor HSVToRGB(float h, float s, float v)
{
    float r = 0, g = 0, b = 0;
    int i   = (int)(h * 6.0f);
    float f = (h * 6.0f) - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - s * f);
    float t = v * (1.0f - s * (1.0f - f));

    switch(i % 6)
    {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        case 5: r = v; g = p; b = q; break;
    }

    return(ToRGBColor((unsigned char)(r * 255.0f), (unsigned char)(g * 255.0f), (unsigned char)(b * 255.0f)));
}

RGBController_HyperXPulsefireRaid::RGBController_HyperXPulsefireRaid(HyperXPulsefireRaidController* controller_ptr)
{
    controller              = controller_ptr;

    name                    = controller->GetNameString();
    vendor                  = "HyperX";
    type                    = DEVICE_TYPE_MOUSE;
    description             = "HyperX Pulsefire Raid Device";
    location                = controller->GetDeviceLocation();
    serial                  = controller->GetSerialString();

    // Mode 0: Direct (Volatile 60 Hz streaming)
    mode Direct;
    Direct.name             = "Direct";
    Direct.value            = 0x00;
    Direct.flags            = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
    Direct.color_mode       = MODE_COLORS_PER_LED;
    Direct.brightness       = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX;
    Direct.brightness_min   = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MIN;
    Direct.brightness_max   = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX;
    modes.push_back(Direct);

    // Mode 1: Static (Autonomous On-Board)
    mode Static;
    Static.name             = "Static";
    Static.value            = HYPERX_PULSEFIRE_RAID_LED_MODE_STATIC;
    Static.flags            = MODE_FLAG_HAS_PER_LED_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_MANUAL_SAVE;
    Static.color_mode       = MODE_COLORS_PER_LED;
    Static.brightness       = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX;
    Static.brightness_min   = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MIN;
    Static.brightness_max   = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX;
    modes.push_back(Static);

    // Mode 2: Breathing (Dual-Color Autonomous Hardware Animation)
    mode Breathing;
    Breathing.name          = "Breathing";
    Breathing.value         = 0x02;
    Breathing.flags         = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_MANUAL_SAVE;
    Breathing.color_mode    = MODE_COLORS_MODE_SPECIFIC;
    Breathing.colors_min    = 1;
    Breathing.colors_max    = 2;
    Breathing.colors.resize(2);
    Breathing.colors[0]     = ToRGBColor(0, 0, 255); // Primary peak
    Breathing.colors[1]     = ToRGBColor(0, 0, 0);   // Secondary baseline / trough
    Breathing.speed         = 50;
    Breathing.speed_min     = 1;
    Breathing.speed_max     = 100;
    Breathing.brightness    = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX;
    Breathing.brightness_min= HYPERX_PULSFIRE_RAID_BRIGHTNESS_MIN;
    Breathing.brightness_max= HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX;
    modes.push_back(Breathing);

    // Mode 3: Spectrum Cycle (Autonomous Hardware Animation)
    mode Cycle;
    Cycle.name              = "Spectrum Cycle";
    Cycle.value             = HYPERX_PULSEFIRE_RAID_LED_MODE_CYCLE;
    Cycle.flags             = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_MANUAL_SAVE;
    Cycle.color_mode        = MODE_COLORS_NONE;
    Cycle.speed             = 50;
    Cycle.speed_min         = 1;
    Cycle.speed_max         = 100;
    Cycle.brightness        = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX;
    Cycle.brightness_min    = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MIN;
    Cycle.brightness_max    = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX;
    modes.push_back(Cycle);

    // Mode 4: Reactive (Hardware Click-Reactive Trigger)
    mode Reactive;
    Reactive.name           = "Reactive";
    Reactive.value          = HYPERX_PULSEFIRE_RAID_LED_MODE_TRIGGER;
    Reactive.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_MANUAL_SAVE;
    Reactive.color_mode     = MODE_COLORS_MODE_SPECIFIC;
    Reactive.colors_min     = 1;
    Reactive.colors_max     = 1;
    Reactive.colors.resize(1);
    Reactive.colors[0]      = ToRGBColor(0, 0, 255);
    Reactive.speed          = 5;
    Reactive.speed_min      = 1;
    Reactive.speed_max      = 9;
    Reactive.brightness     = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX;
    Reactive.brightness_min = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MIN;
    Reactive.brightness_max = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX;
    modes.push_back(Reactive);

    SetupZones();

    // Initialize LED colors from current on-board memory
    unsigned char* cached = controller->GetCachedProfile();
    colors[0] = ToRGBColor(cached[57], cached[58], cached[59]); // Wheel
    colors[1] = ToRGBColor(cached[54], cached[55], cached[56]); // Logo
    if(colors[0] == 0 && colors[1] == 0)
    {
        colors[0] = ToRGBColor(0, 0, 255);
        colors[1] = ToRGBColor(255, 0, 0);
    }

    if(zones.size() > 0 && zones[0].modes.size() > 1 && zones[0].modes[1].colors.size() > 0)
    {
        zones[0].modes[1].colors[0] = colors[0];
    }
    if(zones.size() > 1 && zones[1].modes.size() > 1 && zones[1].modes[1].colors.size() > 0)
    {
        zones[1].modes[1].colors[0] = colors[1];
    }

    direct_slot_active = false;

    keepalive_thread_run = 1;
    keepalive_thread = new std::thread(&RGBController_HyperXPulsefireRaid::KeepaliveThread, this);
}

RGBController_HyperXPulsefireRaid::~RGBController_HyperXPulsefireRaid()
{
    Shutdown();

    keepalive_thread_run = 0;
    if(keepalive_thread && keepalive_thread->joinable())
    {
        keepalive_thread->join();
        delete keepalive_thread;
        keepalive_thread = nullptr;
    }

    // Restore on-board non-volatile EEPROM profile (Slot 1) upon exit
    unsigned char* cached = controller->GetCachedProfile();
    controller->ApplyProfile(cached[14], HYPERX_PULSEFIRE_RAID_SLOT_FLASH);
    direct_slot_active = false;

    delete controller;
}

void RGBController_HyperXPulsefireRaid::SetupZones()
{
    std::string led_names[HYPERX_PULSFIRE_RAID_LEDS_COUNT] =
    {
        "Scroll Wheel", "Logo"
    };

    for(unsigned int i = 0; i < HYPERX_PULSFIRE_RAID_LEDS_COUNT; i++)
    {
        zone new_zone;
        new_zone.name        = led_names[i];
        new_zone.type        = ZONE_TYPE_SINGLE;
        new_zone.leds_min    = 1;
        new_zone.leds_max    = 1;
        new_zone.leds_count  = 1;

        /*-------------------------------------------------*\
        | Populate per-zone modes so SupportsPerZoneModes() |
        | is enabled for independent zone control           |
        \*-------------------------------------------------*/
        mode ZoneDirect;
        ZoneDirect.name             = "Direct";
        ZoneDirect.value            = 0x00;
        ZoneDirect.flags            = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS;
        ZoneDirect.color_mode       = MODE_COLORS_MODE_SPECIFIC;
        ZoneDirect.colors_min       = 1;
        ZoneDirect.colors_max       = 1;
        ZoneDirect.colors.resize(1);
        ZoneDirect.colors[0]        = (i == 0) ? ToRGBColor(0, 0, 255) : ToRGBColor(255, 0, 0);
        ZoneDirect.brightness       = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX;
        ZoneDirect.brightness_min   = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MIN;
        ZoneDirect.brightness_max   = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX;
        new_zone.modes.push_back(ZoneDirect);

        mode ZoneStatic;
        ZoneStatic.name             = "Static";
        ZoneStatic.value            = HYPERX_PULSEFIRE_RAID_LED_MODE_STATIC;
        ZoneStatic.flags            = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_MANUAL_SAVE;
        ZoneStatic.color_mode       = MODE_COLORS_MODE_SPECIFIC;
        ZoneStatic.colors_min       = 1;
        ZoneStatic.colors_max       = 1;
        ZoneStatic.colors.resize(1);
        ZoneStatic.colors[0]        = (i == 0) ? ToRGBColor(0, 0, 255) : ToRGBColor(255, 0, 0);
        ZoneStatic.brightness       = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX;
        ZoneStatic.brightness_min   = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MIN;
        ZoneStatic.brightness_max   = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX;
        new_zone.modes.push_back(ZoneStatic);

        mode ZoneBreathing;
        ZoneBreathing.name          = "Breathing";
        ZoneBreathing.value         = 0x02;
        ZoneBreathing.flags         = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_MANUAL_SAVE;
        ZoneBreathing.color_mode    = MODE_COLORS_MODE_SPECIFIC;
        ZoneBreathing.colors_min    = 1;
        ZoneBreathing.colors_max    = 2;
        ZoneBreathing.colors.resize(2);
        ZoneBreathing.colors[0]     = (i == 0) ? ToRGBColor(0, 0, 255) : ToRGBColor(255, 0, 0);
        ZoneBreathing.colors[1]     = ToRGBColor(0, 0, 0);
        ZoneBreathing.speed         = 50;
        ZoneBreathing.speed_min     = 1;
        ZoneBreathing.speed_max     = 100;
        ZoneBreathing.brightness    = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX;
        ZoneBreathing.brightness_min= HYPERX_PULSFIRE_RAID_BRIGHTNESS_MIN;
        ZoneBreathing.brightness_max= HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX;
        new_zone.modes.push_back(ZoneBreathing);

        mode ZoneCycle;
        ZoneCycle.name              = "Spectrum Cycle";
        ZoneCycle.value             = HYPERX_PULSEFIRE_RAID_LED_MODE_CYCLE;
        ZoneCycle.flags             = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_MANUAL_SAVE;
        ZoneCycle.color_mode        = MODE_COLORS_NONE;
        ZoneCycle.speed             = 50;
        ZoneCycle.speed_min         = 1;
        ZoneCycle.speed_max         = 100;
        ZoneCycle.brightness        = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX;
        ZoneCycle.brightness_min    = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MIN;
        ZoneCycle.brightness_max    = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX;
        new_zone.modes.push_back(ZoneCycle);

        mode ZoneReactive;
        ZoneReactive.name           = "Reactive";
        ZoneReactive.value          = HYPERX_PULSEFIRE_RAID_LED_MODE_TRIGGER;
        ZoneReactive.flags          = MODE_FLAG_HAS_SPEED | MODE_FLAG_HAS_MODE_SPECIFIC_COLOR | MODE_FLAG_HAS_BRIGHTNESS | MODE_FLAG_MANUAL_SAVE;
        ZoneReactive.color_mode     = MODE_COLORS_MODE_SPECIFIC;
        ZoneReactive.colors_min     = 1;
        ZoneReactive.colors_max     = 1;
        ZoneReactive.colors.resize(1);
        ZoneReactive.colors[0]      = (i == 0) ? ToRGBColor(0, 0, 255) : ToRGBColor(255, 0, 0);
        ZoneReactive.speed          = 5;
        ZoneReactive.speed_min      = 1;
        ZoneReactive.speed_max      = 9;
        ZoneReactive.brightness     = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX;
        ZoneReactive.brightness_min = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MIN;
        ZoneReactive.brightness_max = HYPERX_PULSFIRE_RAID_BRIGHTNESS_MAX;
        new_zone.modes.push_back(ZoneReactive);

        zones.push_back(new_zone);

        led new_led;
        new_led.name         = led_names[i];
        leds.push_back(new_led);
    }

    SetupColors();
}

void RGBController_HyperXPulsefireRaid::DeviceUpdateLEDs()
{
    if(modes[active_mode].name == "Direct")
    {
        if(!direct_slot_active.load())
        {
            unsigned char* cached = controller->GetCachedProfile();
            unsigned char brightness = (modes[active_mode].brightness > 0) ? modes[active_mode].brightness : cached[14];
            controller->ApplyProfile(brightness, HYPERX_PULSEFIRE_RAID_SLOT_RUNTIME);
            direct_slot_active = true;
        }

        last_update_time = std::chrono::steady_clock::now();
        controller->SendColors(colors);
    }
    else
    {
        direct_slot_active = false;
        SendHardwareConfig(HYPERX_PULSEFIRE_RAID_SLOT_RUNTIME);
    }
}

void RGBController_HyperXPulsefireRaid::DeviceUpdateZoneLEDs(int /*zone*/)
{
    DeviceUpdateLEDs();
}

void RGBController_HyperXPulsefireRaid::DeviceUpdateSingleLED(int /*led*/)
{
    DeviceUpdateLEDs();
}

void RGBController_HyperXPulsefireRaid::DeviceUpdateMode()
{
    if(modes[active_mode].name == "Direct")
    {
        unsigned char* cached = controller->GetCachedProfile();
        unsigned char brightness = (modes[active_mode].brightness > 0) ? modes[active_mode].brightness : cached[14];
        controller->ApplyProfile(brightness, HYPERX_PULSEFIRE_RAID_SLOT_RUNTIME);
        direct_slot_active = true;
    }
    else
    {
        direct_slot_active = false;
    }

    for(std::size_t zone_idx = 0; zone_idx < zones.size(); zone_idx++)
    {
        if(active_mode < (int)zones[zone_idx].modes.size())
        {
            zones[zone_idx].active_mode = active_mode;

            // Sync colors from global mode if available
            if(modes[active_mode].colors.size() > 0 && zones[zone_idx].modes[active_mode].colors.size() > 0)
            {
                zones[zone_idx].modes[active_mode].colors[0] = modes[active_mode].colors[0];
                if(modes[active_mode].colors.size() > 1 && zones[zone_idx].modes[active_mode].colors.size() > 1)
                {
                    zones[zone_idx].modes[active_mode].colors[1] = modes[active_mode].colors[1];
                }
            }
            zones[zone_idx].modes[active_mode].speed      = modes[active_mode].speed;
            zones[zone_idx].modes[active_mode].brightness = modes[active_mode].brightness;
        }
    }

    DeviceUpdateLEDs();
}

void RGBController_HyperXPulsefireRaid::DeviceUpdateZoneMode(int zone)
{
    if(zone >= 0 && (std::size_t)zone < zones.size())
    {
        int z_mode = zones[zone].active_mode;
        if(z_mode >= 0 && z_mode < (int)zones[zone].modes.size())
        {
            if(zones[zone].modes[z_mode].name == "Static" && zones[zone].modes[z_mode].colors.size() > 0)
            {
                colors[zone] = zones[zone].modes[z_mode].colors[0];
            }
        }
    }

    DeviceUpdateLEDs();
}

void RGBController_HyperXPulsefireRaid::DeviceSaveMode()
{
    SendHardwareConfig(HYPERX_PULSEFIRE_RAID_SLOT_FLASH);
}

void RGBController_HyperXPulsefireRaid::SendHardwareConfig(unsigned char slot)
{
    if(modes[active_mode].name == "Direct")
    {
        last_update_time = std::chrono::steady_clock::now();
        controller->SendColors(colors);
        return;
    }

    mode* mode_ptr[2] = { nullptr, nullptr };
    for(unsigned int i = 0; i < 2; i++)
    {
        if((zones[i].active_mode >= 0) && (zones[i].active_mode < (int)zones[i].modes.size()))
        {
            mode_ptr[i] = &zones[i].modes[zones[i].active_mode];
        }
        else if(active_mode < (int)modes.size())
        {
            mode_ptr[i] = &modes[active_mode];
        }
    }

    if(!mode_ptr[0] || !mode_ptr[1])
    {
        return;
    }

    // Default primary and secondary colors (anchored to per-LED colors)
    RGBColor w_prim = colors[0];
    RGBColor w_sec  = ToRGBColor(0, 0, 0);

    RGBColor l_prim = colors[1];
    RGBColor l_sec  = ToRGBColor(0, 0, 0);

    // Resolve Scroll Wheel (Zone 0)
    if(mode_ptr[0]->name == "Breathing")
    {
        w_prim = (mode_ptr[0]->colors.size() > 0) ? mode_ptr[0]->colors[0] : colors[0];
        w_sec  = (mode_ptr[0]->colors.size() > 1) ? mode_ptr[0]->colors[1] : ToRGBColor(0, 0, 0);
    }
    else if(mode_ptr[0]->name == "Reactive")
    {
        w_prim = (mode_ptr[0]->colors.size() > 0) ? mode_ptr[0]->colors[0] : colors[0];
    }
    else if(mode_ptr[0]->name == "Static")
    {
        w_prim = colors[0];
        if(zones[0].modes.size() > 1 && zones[0].modes[1].colors.size() > 0)
        {
            zones[0].modes[1].colors[0] = w_prim;
        }
    }

    // Resolve Logo (Zone 1)
    if(mode_ptr[1]->name == "Breathing")
    {
        l_prim = (mode_ptr[1]->colors.size() > 0) ? mode_ptr[1]->colors[0] : colors[1];
        l_sec  = (mode_ptr[1]->colors.size() > 1) ? mode_ptr[1]->colors[1] : ToRGBColor(0, 0, 0);
    }
    else if(mode_ptr[1]->name == "Reactive")
    {
        l_prim = (mode_ptr[1]->colors.size() > 0) ? mode_ptr[1]->colors[0] : colors[1];
    }
    else if(mode_ptr[1]->name == "Static")
    {
        l_prim = colors[1];
        if(zones[1].modes.size() > 1 && zones[1].modes[1].colors.size() > 0)
        {
            zones[1].modes[1].colors[0] = l_prim;
        }
    }

    unsigned char profile[256];
    memcpy(profile, controller->GetCachedProfile(), 256);

    unsigned char brightness = (unsigned char)((mode_ptr[0]->brightness + mode_ptr[1]->brightness) / 2);
    profile[14] = brightness;

    // Trigger pulse speeds (1 to 9)
    unsigned char w_trig_speed = (mode_ptr[0]->speed >= 1 && mode_ptr[0]->speed <= 9) ? (unsigned char)mode_ptr[0]->speed : 5;
    unsigned char l_trig_speed = (mode_ptr[1]->speed >= 1 && mode_ptr[1]->speed <= 9) ? (unsigned char)mode_ptr[1]->speed : 5;

    if(mode_ptr[0]->name == "Reactive")
    {
        profile[48] = HYPERX_PULSEFIRE_RAID_LED_MODE_TRIGGER;
        profile[89] = w_trig_speed;
        profile[75] = RGBGetRValue(w_prim);
        profile[76] = RGBGetGValue(w_prim);
        profile[77] = RGBGetBValue(w_prim);
    }
    else
    {
        profile[48] = HYPERX_PULSEFIRE_RAID_LED_MODE_SYNC;
    }

    if(mode_ptr[1]->name == "Reactive")
    {
        profile[47] = HYPERX_PULSEFIRE_RAID_LED_MODE_TRIGGER;
        profile[91] = l_trig_speed;
        profile[78] = RGBGetRValue(l_prim);
        profile[79] = RGBGetGValue(l_prim);
        profile[80] = RGBGetBValue(l_prim);
    }
    else
    {
        profile[47] = HYPERX_PULSEFIRE_RAID_LED_MODE_SYNC;
    }

    // Static colors in profile registers
    profile[54] = RGBGetRValue(l_prim);
    profile[55] = RGBGetGValue(l_prim);
    profile[56] = RGBGetBValue(l_prim);
    profile[57] = RGBGetRValue(w_prim);
    profile[58] = RGBGetGValue(w_prim);
    profile[59] = RGBGetBValue(w_prim);

    profile[96] = 0; // TriggerMode: Solid

    bool is_animated = (mode_ptr[0]->name == "Breathing" || mode_ptr[0]->name == "Spectrum Cycle" ||
                        mode_ptr[1]->name == "Breathing" || mode_ptr[1]->name == "Spectrum Cycle");

    const unsigned int frame_count = (is_animated) ? 30 : 1;
    unsigned char frame_buffer[frame_count * 6];

    unsigned char interval_ms = 50;
    if(is_animated)
    {
        unsigned int avg_speed = (mode_ptr[0]->speed + mode_ptr[1]->speed) / 2;
        if(avg_speed < 1)   avg_speed = 1;
        if(avg_speed > 100) avg_speed = 100;

        interval_ms = (unsigned char)(100 - (avg_speed * 80 / 100));
        if(interval_ms < 20)  interval_ms = 20;
        if(interval_ms > 100) interval_ms = 100;
    }

    for(unsigned int f = 0; f < frame_count; f++)
    {
        // Scroll Wheel (Zone 0, Row 0 in frame buffer)
        RGBColor w_col = 0;
        if(mode_ptr[0]->name == "Breathing")
        {
            float factor = (sinf((f / 30.0f) * 2.0f * (float)M_PI - ((float)M_PI / 2.0f)) + 1.0f) / 2.0f;
            unsigned char r = (unsigned char)((1.0f - factor) * RGBGetRValue(w_sec) + factor * RGBGetRValue(w_prim));
            unsigned char g = (unsigned char)((1.0f - factor) * RGBGetGValue(w_sec) + factor * RGBGetGValue(w_prim));
            unsigned char b = (unsigned char)((1.0f - factor) * RGBGetBValue(w_sec) + factor * RGBGetBValue(w_prim));
            w_col = ToRGBColor(r, g, b);
        }
        else if(mode_ptr[0]->name == "Spectrum Cycle")
        {
            w_col = HSVToRGB(f / 30.0f, 1.0f, 1.0f);
        }
        else if(mode_ptr[0]->name == "Static" || mode_ptr[0]->name == "Direct")
        {
            w_col = w_prim;
        }
        else // Reactive idle
        {
            w_col = 0;
        }

        frame_buffer[f * 6 + 0] = RGBGetRValue(w_col);
        frame_buffer[f * 6 + 1] = RGBGetGValue(w_col);
        frame_buffer[f * 6 + 2] = RGBGetBValue(w_col);

        // Logo (Zone 1, Row 1 in frame buffer)
        RGBColor l_col = 0;
        if(mode_ptr[1]->name == "Breathing")
        {
            float factor = (sinf((f / 30.0f) * 2.0f * (float)M_PI - ((float)M_PI / 2.0f)) + 1.0f) / 2.0f;
            unsigned char r = (unsigned char)((1.0f - factor) * RGBGetRValue(l_sec) + factor * RGBGetRValue(l_prim));
            unsigned char g = (unsigned char)((1.0f - factor) * RGBGetGValue(l_sec) + factor * RGBGetGValue(l_prim));
            unsigned char b = (unsigned char)((1.0f - factor) * RGBGetBValue(l_sec) + factor * RGBGetBValue(l_prim));
            l_col = ToRGBColor(r, g, b);
        }
        else if(mode_ptr[1]->name == "Spectrum Cycle")
        {
            l_col = HSVToRGB(f / 30.0f, 1.0f, 1.0f);
        }
        else if(mode_ptr[1]->name == "Static" || mode_ptr[1]->name == "Direct")
        {
            l_col = l_prim;
        }
        else // Reactive idle
        {
            l_col = 0;
        }

        frame_buffer[f * 6 + 3] = RGBGetRValue(l_col);
        frame_buffer[f * 6 + 4] = RGBGetGValue(l_col);
        frame_buffer[f * 6 + 5] = RGBGetBValue(l_col);
    }

    profile[112] = (unsigned char)frame_count;
    profile[113] = interval_ms;

    // Only update in-memory cache baseline when explicitly burning to EEPROM
    if(slot == HYPERX_PULSEFIRE_RAID_SLOT_FLASH)
    {
        direct_slot_active = false;
        memcpy(controller->GetCachedProfile(), profile, 256);
    }

    controller->UploadAnimationFrames(frame_buffer, frame_count, slot);
    controller->SaveProfile(profile, brightness, slot);
}

void RGBController_HyperXPulsefireRaid::KeepaliveThread()
{
    while(keepalive_thread_run.load())
    {
        if(active_mode == 0 && direct_slot_active.load()) // Direct Mode only when active
        {
            if((std::chrono::steady_clock::now() - last_update_time) > 1s)
            {
                UpdateLEDsInternal();
            }
        }

        std::this_thread::sleep_for(20ms);
    }
}
