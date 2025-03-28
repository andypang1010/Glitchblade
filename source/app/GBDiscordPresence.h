#pragma once

#include <cstdint>
#include <iostream>
#include <thread>
#include <atomic>
#include <cstring>
#include <cugl/cugl.h>

using namespace cugl;

#if __has_include("../../extern/discord_social_sdk/include/cdiscord.h")
#include "cdiscord.h"
#define DISCORD_SDK_AVAILABLE 1
#else
#define DISCORD_SDK_AVAILABLE 0
#endif

#if DISCORD_SDK_AVAILABLE

class GlitchbladeDiscordPresence {
public:
    static inline void startPresence() {
        // Replace with your actual Discord Application ID (as a string!)
        constexpr const char* CLIENT_ID = "1342953286918996038";

        int result = discord_initialize(CLIENT_ID, true);  // auto register = true
        if (result != 0) {
            CULog("Failed to initialize Discord Social SDK!");
            return;
        }

        CULog("Initialized Discord Social SDK.");

        discord_activity_t activity{};
        std::strncpy(activity.state, "Playing", sizeof(activity.state));
        std::strncpy(activity.details, "In a Game", sizeof(activity.details));
        std::strncpy(activity.assets.large_image, "gbtemplogo", sizeof(activity.assets.large_image));
        std::strncpy(activity.assets.large_text, "Glitchblade", sizeof(activity.assets.large_text));

        discord_update_activity(&activity, [](int result) {
            if (result == 0) {
                CULog("Updated Discord Rich Presence successfully.");
            }
            else {
                CULog("Failed to update Discord Rich Presence!");
            }
            });

        running = true;
        presenceThread = std::thread([]() {
            while (running) {
                discord_run_callbacks(); // Required to keep SDK active
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
            });
    }

    static inline void stopPresence() {
        if (running) {
            CULog("Stopping Discord Rich Presence...");
            running = false;
            if (presenceThread.joinable()) {
                presenceThread.join();
            }
            discord_shutdown();
            CULog("Discord Rich Presence shut down.");
        }
    }

private:
    static inline std::thread presenceThread;
    static inline std::atomic<bool> running = false;
};

#endif
