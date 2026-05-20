#pragma once

#include <string>
#include <mutex>
#include <mysql.h>

namespace PlayerTracker {

    struct PlayerEvent {
        std::string event_type;    // "join" ou "leave"
        std::string server_name;
        std::string eos_id;
        std::string player_name;
        double x = 0, y = 0, z = 0;
        bool has_position = false;
    };

    class Database {
    public:
        Database();
        ~Database();

        bool Init(const std::string& host,
                  unsigned int       port,
                  const std::string& user,
                  const std::string& password,
                  const std::string& dbname,
                  const std::string& table);

        void Shutdown();

        // Insère un évènement (join/leave) avec ou sans position
        bool Insert(const PlayerEvent& evt);

    private:
        bool EnsureConnection();
        std::string Escape(const std::string& in);

        MYSQL*       conn_ = nullptr;
        std::string  host_;
        unsigned int port_ = 3306;
        std::string  user_, password_, dbname_, table_;
        std::mutex   mtx_;
    };
}
