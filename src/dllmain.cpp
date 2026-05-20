#include <Windows.h>
#include "Plugin.h"
#include "Logger.h"

namespace PlayerTracker {
    void RegisterHooks();
    void UnregisterHooks();
}

BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID) {
    (void)reason;
    return TRUE;
}

extern "C" __declspec(dllexport) void Plugin_Init() {
    PTLog::Info("Plugin_Init: démarrage PlayerTracker");
    if (!PlayerTracker::Plugin::Get().Load()) {
        PTLog::Error("Plugin_Init: échec.");
        return;
    }
    PlayerTracker::RegisterHooks();
    PTLog::Info("Plugin_Init: prêt.");
}

extern "C" __declspec(dllexport) void Plugin_Unload() {
    PTLog::Info("Plugin_Unload");
    PlayerTracker::UnregisterHooks();
    PlayerTracker::Plugin::Get().Unload();
}
