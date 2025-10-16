#pragma once
#include <obs-module.h>

struct PluginSettings {
    float zoomSpeed = 1.0f;
    float panSpeed  = 1.0f;
    float deadzone  = 0.15f;
    bool  enabled   = false;
};

extern PluginSettings g_settings;

void load_plugin_settings();
void save_plugin_settings();
