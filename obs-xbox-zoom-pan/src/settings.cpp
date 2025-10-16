#include "settings.hpp"

PluginSettings g_settings;

void load_plugin_settings()
{
    obs_data_t* data = obs_load_module_config_file(obs_current_module());
    if (!data) return;

    if (obs_data_has_user_value(data, "zoomSpeed"))
        g_settings.zoomSpeed = (float)obs_data_get_double(data, "zoomSpeed");
    if (obs_data_has_user_value(data, "panSpeed"))
        g_settings.panSpeed  = (float)obs_data_get_double(data, "panSpeed");
    if (obs_data_has_user_value(data, "deadzone"))
        g_settings.deadzone  = (float)obs_data_get_double(data, "deadzone");
    if (obs_data_has_user_value(data, "enabled"))
        g_settings.enabled   = obs_data_get_bool(data, "enabled");

    obs_data_release(data);
}

void save_plugin_settings()
{
    obs_data_t* data = obs_data_create();
    obs_data_set_double(data, "zoomSpeed", g_settings.zoomSpeed);
    obs_data_set_double(data, "panSpeed",  g_settings.panSpeed);
    obs_data_set_double(data, "deadzone",  g_settings.deadzone);
    obs_data_set_bool(data, "enabled",     g_settings.enabled);

    obs_save_module_config_file(obs_current_module(), data);
    obs_data_release(data);
}
