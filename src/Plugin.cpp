#include "Plugin.h"
#include "Logger.h"

#include <fstream>
#include <filesystem>
#include "json.hpp"

namespace PlayerTracker {

    using json = nlohmann::json;

    Plugin& Plugin::Get() { static Plugin i; return i; }

    static std::filesystem::path ConfigPath() {
        return std::filesystem::current_path()
            / "ArkApi" / "Plugins" / "PlayerTracker" / "config.json";
    }

    bool Plugin::Load() {
        if (loaded_) return true;
        PTLog::Info("Chargement PlayerTracker...");

        const auto path = ConfigPath();
        if (!std::filesystem::exists(path)) {
            PTLog::Error("config.json introuvable: %s", path.string().c_str());
            return false;
        }

        try {
            std::ifstream f(path);
            json j; f >> j;
            cfg_.mysql_host     = j.value("/MySQL/Host"_json_pointer,     cfg_.mysql_host);
            cfg_.mysql_port     = j.value("/MySQL/Port"_json_pointer,     cfg_.mysql_port);
            cfg_.mysql_user     = j.value("/MySQL/User"_json_pointer,     cfg_.mysql_user);
            cfg_.mysql_password = j.value("/MySQL/Password"_json_pointer, cfg_.mysql_password);
            cfg_.mysql_database = j.value("/MySQL/Database"_json_pointer, cfg_.mysql_database);
            cfg_.mysql_table    = j.value("/MySQL/Table"_json_pointer,    cfg_.mysql_table);
            cfg_.server_name    = j.value("/ServerName"_json_pointer,     cfg_.server_name);
        }
        catch (const std::exception& e) {
            PTLog::Error("Parse config: %s", e.what());
            return false;
        }

        if (!db_.Init(cfg_.mysql_host, cfg_.mysql_port, cfg_.mysql_user,
                      cfg_.mysql_password, cfg_.mysql_database, cfg_.mysql_table)) {
            return false;
        }

        loaded_ = true;
        PTLog::Info("PlayerTracker chargé (server_name=%s)", cfg_.server_name.c_str());
        return true;
    }

    void Plugin::Unload() {
        if (!loaded_) return;
        db_.Shutdown();
        loaded_ = false;
        PTLog::Info("PlayerTracker déchargé.");
    }

    std::string Plugin::GetEOSID(AShooterPlayerController* controller) {
        if (!controller) return "";
        FString eos = AsaApi::GetApiUtils().GetEOSIDFromController(controller);
        return std::string(TCHAR_TO_UTF8(*eos));
    }

    std::string Plugin::GetPlayerName(AShooterPlayerController* controller) {
        if (!controller) return "";
        FString name;
        controller->GetPlayerCharacterName(&name);
        return std::string(TCHAR_TO_UTF8(*name));
    }

    bool Plugin::GetPosition(APlayerController* pc, double& x, double& y, double& z) {
        if (!pc) return false;

        // Helper local: extrait la position depuis un acteur via son RootComponent.
        auto extractFromActor = [&](AActor* actor) -> bool {
            if (!actor) return false;
            USceneComponent* root = actor->RootComponentField().Get();
            if (!root) return false;
            UE::Math::TVector<double>& loc = root->RelativeLocationField();
            x = loc.X; y = loc.Y; z = loc.Z;
            return true;
        };

        // Tentative 1: depuis le pawn (character du joueur)
        APawn* pawn = pc->PawnField().Get();
        if (extractFromActor(pawn)) return true;

        // Tentative 2: depuis le controller lui-même
        if (extractFromActor(pc)) return true;

        return false;
    }
}
