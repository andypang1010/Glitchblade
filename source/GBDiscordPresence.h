#pragma once

#include <cstdint>
#include <iostream>
#include <thread>
#include <atomic>
#include <cugl/cugl.h>

using namespace cugl;

/*
    If you are going to move the discordsdk into the source folder, make sure you follow the instructions here so the linker doesn't whine and throw a tantrum over a missing .dll.lib or .so, etc:
    https://discord.com/developers/docs/developer-tools/game-sdk

    For example, on Windows x64, I put the .dll.lib and the .dll file in \Glitchblade\build\windows\x64\Debug and added discord_game_sdk.dll.lib to my Project Properties > Linker > Input > Additional Dependencies

    The project will NOT build if you include the discordsdk in the source folder when building UNLESS you have the necessary library, dll, etc. available.
    For me, the .dll and .dll.lib went in \Glitchblade\build\windows\x64\Debug
*/

#if __has_include("discordsdk/discord.h")
    #include "discordsdk/discord.h"
    #define DISCORD_SDK_AVAILABLE 1
#else
    #define DISCORD_SDK_AVAILABLE 0
#endif

#if DISCORD_SDK_AVAILABLE

class GlitchbladeDiscordPresence {
public:
    static inline void startPresence() {
        discord::Core* core{};
        auto result = discord::Core::Create(1342953286918996038, DiscordCreateFlags_Default, &core);
        if (result != discord::Result::Ok) {
            CULog("Failed to initialize Discord SDK!");
            return;
        }

        discord::Activity activity{};
        activity.SetState("Playing");
        activity.SetDetails("In a Game");
        activity.GetAssets().SetLargeImage("gbtemplogo");
        activity.GetAssets().SetLargeText("Glitchblade");

        core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {
            if (result == discord::Result::Ok) {
                CULog("Updated Discord Rich Presence successfully.");
            }
            else {
                CULog("Failed to update Discord Rich Presence!");
            }
            });

        discordCore = core;
        running = true;

        // New thread so we don't block the main thread
        presenceThread = std::thread([]() {
            while (running) {
                if (discordCore) {
                    discordCore->RunCallbacks();
                }
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
            });
    }

    static inline void stopPresence() {
        if (discordCore) {
            CULog("Stopping Discord Rich Presence...");
            running = false;
            if (presenceThread.joinable()) {
                presenceThread.join();
            }
            delete discordCore;
            discordCore = nullptr;
        }
    }

private:
    static inline discord::Core* discordCore = nullptr;
    static inline std::thread presenceThread;
    static inline std::atomic<bool> running = false;
};

#endif
