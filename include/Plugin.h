#pragma once

#include <string>
#include "Database.h"
#include "API/ARK/Ark.h"

namespace PlayerTracker {

    struct Config {
        std::string  mysql_host     = "127.0.0.1";
        unsigned int mysql_port     = 3306;
        std::string  mysql_user     = "ark_ban";
        std::string  mysql_password = "changeme";
        std::string  mysql_database = "ark_bans";
        std::string  mysql_table    = "player_events";
        std::string  server_name    = "Unknown";
    };

    class Plugin {
    public:
        static Plugin& Get();

        bool Load();
        void Unload();

        Database& Db() { return db_; }
        const Config& Cfg() const { return cfg_; }

        // Helpers
        static std::string GetEOSID(AShooterPlayerController* controller);
        static std::string GetPlayerName(AShooterPlayerController* controller);
        // Capture la position du pawn. Retourne false si le pawn n'a pas spawn.
        static bool GetPosition(APlayerController* pc, double& x, double& y, double& z);

    private:
        Plugin() = default;
        Database db_;
        Config   cfg_;
        bool     loaded_ = false;
    };
}
