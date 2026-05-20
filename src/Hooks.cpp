#include "Plugin.h"
#include "Logger.h"
#include "API/ARK/Ark.h"

namespace PlayerTracker {

    // ---------- Hook PostLogin (connexion) ----------
    DECLARE_HOOK(AShooterGameMode_PostLogin,
                 void,
                 AShooterGameMode*,
                 APlayerController*);

    void Hook_AShooterGameMode_PostLogin(AShooterGameMode* gm,
                                         APlayerController* pc_base) {
        AShooterGameMode_PostLogin_original(gm, pc_base);

        auto* pc = static_cast<AShooterPlayerController*>(pc_base);
        if (!pc) return;

        PlayerEvent evt;
        evt.event_type  = "join";
        evt.server_name = Plugin::Get().Cfg().server_name;
        evt.eos_id      = Plugin::GetEOSID(pc);
        evt.player_name = Plugin::GetPlayerName(pc);

        evt.has_position = Plugin::GetPosition(pc_base, evt.x, evt.y, evt.z);

        if (evt.eos_id.empty()) {
            PTLog::Warn("PostLogin sans EOSID, skip");
            return;
        }

        Plugin::Get().Db().Insert(evt);
        PTLog::Info("Join: %s (%s) pos=%.0f,%.0f,%.0f",
                    evt.player_name.c_str(), evt.eos_id.c_str(),
                    evt.x, evt.y, evt.z);
    }

    // ---------- Hook Logout (déconnexion) ----------
    // Signature UE: void AGameModeBase::Logout(AController* Exiting)
    DECLARE_HOOK(AShooterGameMode_Logout,
                 void,
                 AShooterGameMode*,
                 AController*);

    void Hook_AShooterGameMode_Logout(AShooterGameMode* gm, AController* exiting) {
        // CAPTURE AVANT l'appel original — le pawn est encore valide ici
        auto* pc = static_cast<AShooterPlayerController*>(exiting);
        if (pc) {
            PlayerEvent evt;
            evt.event_type  = "leave";
            evt.server_name = Plugin::Get().Cfg().server_name;
            evt.eos_id      = Plugin::GetEOSID(pc);
            evt.player_name = Plugin::GetPlayerName(pc);
            evt.has_position = Plugin::GetPosition(pc, evt.x, evt.y, evt.z);

            if (!evt.eos_id.empty()) {
                Plugin::Get().Db().Insert(evt);
                PTLog::Info("Leave: %s (%s) pos=%.0f,%.0f,%.0f",
                            evt.player_name.c_str(), evt.eos_id.c_str(),
                            evt.x, evt.y, evt.z);
            }
        }

        AShooterGameMode_Logout_original(gm, exiting);
    }

    // ---------- Register / Unregister ----------
    static constexpr const char* kPostLoginName =
        "AShooterGameMode.PostLogin(APlayerController*)";
    static constexpr const char* kLogoutName =
        "AShooterGameMode.Logout(AController*)";

    void RegisterHooks() {
        AsaApi::GetHooks().SetHook(kPostLoginName,
                                   &Hook_AShooterGameMode_PostLogin,
                                   &AShooterGameMode_PostLogin_original);
        AsaApi::GetHooks().SetHook(kLogoutName,
                                   &Hook_AShooterGameMode_Logout,
                                   &AShooterGameMode_Logout_original);
    }

    void UnregisterHooks() {
        AsaApi::GetHooks().DisableHook(kPostLoginName, &Hook_AShooterGameMode_PostLogin);
        AsaApi::GetHooks().DisableHook(kLogoutName,    &Hook_AShooterGameMode_Logout);
    }
}
