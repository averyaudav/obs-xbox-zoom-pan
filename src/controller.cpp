#include <SDL.h>
#include <atomic>
#include <thread>
#include <vector>
#include <algorithm>
#include <cmath>
#include <chrono>
#include "settings.hpp"

// Forward from transformer.cpp
extern void on_controller_input(float zoom, float panX, float panY, bool reset);

static std::thread s_thread;
static std::atomic<bool> s_running{false};

void start_controller_thread()
{
    if (s_running.exchange(true)) return;

    s_thread = std::thread([] {
        SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS);
        std::vector<SDL_GameController*> pads;

        auto try_add = [&](int idx){
            if (SDL_IsGameController(idx)) {
                SDL_GameController* gc = SDL_GameControllerOpen(idx);
                if (gc) pads.push_back(gc);
            }
        };

        // Initial open of any existing controllers
        for (int i = 0; i < SDL_NumJoysticks(); ++i) try_add(i);

        while (s_running) {
            SDL_Event e;
            while (SDL_PollEvent(&e)) {
                if (e.type == SDL_CONTROLLERDEVICEADDED)    try_add(e.cdevice.which);
                if (e.type == SDL_CONTROLLERDEVICEREMOVED) {
                    for (auto it = pads.begin(); it != pads.end();) {
                        SDL_JoystickID id = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(*it));
                        if (id == e.cdevice.which) {
                            SDL_GameControllerClose(*it);
                            it = pads.erase(it);
                        } else ++it;
                    }
                }
            }

            for (auto* pad : pads) {
                float ly = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_LEFTY)  / 32768.0f;
                float rx = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_RIGHTX) / 32768.0f;
                float ry = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_RIGHTY) / 32768.0f;
                float rt = SDL_GameControllerGetAxis(pad, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0f;

                auto dead = [](float v, float d){
                    float a = std::fabs(v);
                    if (a < d) return 0.0f;
                    return std::copysign((a - d) / (1.0f - d), v);
                };

                ly = dead(ly, g_settings.deadzone);
                rx = dead(rx, g_settings.deadzone);
                ry = dead(ry, g_settings.deadzone);

                on_controller_input(-ly * g_settings.zoomSpeed,
                                     rx * g_settings.panSpeed,
                                    -ry * g_settings.panSpeed,
                                     rt > 0.9f);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(16));
        }

        for (auto* p : pads) SDL_GameControllerClose(p);
        SDL_Quit();
    });
}

void stop_controller_thread()
{
    if (!s_running.exchange(false)) return;
    if (s_thread.joinable()) s_thread.join();
}
