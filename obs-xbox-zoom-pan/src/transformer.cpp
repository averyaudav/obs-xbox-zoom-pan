#include <obs.h>
#include <obs-frontend-api.h>
#include <algorithm>
#include <cmath>
#include "settings.hpp"

// Target item & smoothed state
static obs_sceneitem_t* s_target = nullptr;
static vec2 s_pos{0.0f, 0.0f};
static vec2 s_scale{1.0f, 1.0f};
static bool s_haveState = false;

static inline float clampf(float v, float lo, float hi) {
    return std::max(lo, std::min(hi, v));
}

void set_target_source(const char* name)
{
    obs_source_t* scene = obs_frontend_get_current_scene();
    if (!scene) return;

    obs_scene_t* s = obs_scene_from_source(scene);
    obs_sceneitem_t* it = obs_scene_find_source(s, name);

    if (it) obs_sceneitem_addref(it);
    if (s_target) obs_sceneitem_release(s_target);
    s_target = it;

    s_haveState = false;
    obs_source_release(scene);
}

void set_enabled(bool e)         { g_settings.enabled = e; save_plugin_settings(); }
void set_speeds(float z, float p){ g_settings.zoomSpeed = z; g_settings.panSpeed = p; save_plugin_settings(); }
void set_deadzone(float d)       { g_settings.deadzone = d; save_plugin_settings(); }

void on_controller_input(float zoom, float panX, float panY, bool reset)
{
    if (!g_settings.enabled || !s_target) return;

    if (!s_haveState) {
        obs_sceneitem_get_pos(s_target, &s_pos);
        obs_sceneitem_get_scale(s_target, &s_scale);
        s_haveState = true;
    }

    if (reset) {
        s_pos = {0.0f, 0.0f};
        s_scale = {1.0f, 1.0f};
    } else {
        float z = 1.0f + zoom * 0.02f;
        s_scale.x *= z; s_scale.y *= z;
        s_pos.x += panX; s_pos.y += panY;
    }

    // Clamp zoom to reasonable range
    s_scale.x = clampf(s_scale.x, 0.25f, 4.0f);
    s_scale.y = clampf(s_scale.y, 0.25f, 4.0f);

    // Smoothly lerp current towards target (butter)
    vec2 curPos, curScale;
    obs_sceneitem_get_pos(s_target, &curPos);
    obs_sceneitem_get_scale(s_target, &curScale);

    const float lerp = 0.2f;
    curPos.x   += (s_pos.x   - curPos.x)   * lerp;
    curPos.y   += (s_pos.y   - curPos.y)   * lerp;
    curScale.x += (s_scale.x - curScale.x) * lerp;
    curScale.y += (s_scale.y - curScale.y) * lerp;

    obs_sceneitem_set_pos(s_target, &curPos);
    obs_sceneitem_set_scale(s_target, &curScale);
}
