#include "Database.h"
#include "Logger.h"

#include <chrono>
#include <sstream>

namespace PlayerTracker {

    Database::Database() = default;
    Database::~Database() { Shutdown(); }

    bool Database::Init(const std::string& host, unsigned int port,
                        const std::string& user, const std::string& password,
                        const std::string& dbname, const std::string& table) {
        std::lock_guard<std::mutex> lk(mtx_);
        host_ = host; port_ = port; user_ = user;
        password_ = password; dbname_ = dbname; table_ = table;

        conn_ = mysql_init(nullptr);
        if (!conn_) { PTLog::Error("mysql_init null"); return false; }
        bool reconnect = true;
        mysql_options(conn_, MYSQL_OPT_RECONNECT, &reconnect);
        mysql_options(conn_, MYSQL_SET_CHARSET_NAME, "utf8mb4");

        if (!mysql_real_connect(conn_, host_.c_str(), user_.c_str(),
                                password_.c_str(), dbname_.c_str(),
                                port_, nullptr, 0)) {
            PTLog::Error("Connexion MySQL échouée: %s", mysql_error(conn_));
            mysql_close(conn_); conn_ = nullptr;
            return false;
        }

        std::ostringstream q;
        q << "CREATE TABLE IF NOT EXISTS `" << table_ << "` ("
          << " `id` BIGINT PRIMARY KEY AUTO_INCREMENT,"
          << " `event_type` VARCHAR(10) NOT NULL,"
          << " `server_name` VARCHAR(64) NOT NULL,"
          << " `eos_id` VARCHAR(64) NOT NULL,"
          << " `player_name` VARCHAR(128) NOT NULL,"
          << " `pos_x` DOUBLE NULL,"
          << " `pos_y` DOUBLE NULL,"
          << " `pos_z` DOUBLE NULL,"
          << " `created_at` BIGINT NOT NULL,"
          << " INDEX `idx_created` (`created_at`),"
          << " INDEX `idx_eos` (`eos_id`),"
          << " INDEX `idx_server` (`server_name`)"
          << ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;";
        if (mysql_query(conn_, q.str().c_str()) != 0) {
            PTLog::Error("CREATE TABLE échoué: %s", mysql_error(conn_));
            return false;
        }
        PTLog::Info("DB OK, table `%s` prête", table_.c_str());
        return true;
    }

    void Database::Shutdown() {
        std::lock_guard<std::mutex> lk(mtx_);
        if (conn_) { mysql_close(conn_); conn_ = nullptr; }
    }

    bool Database::EnsureConnection() {
        if (!conn_) return false;
        if (mysql_ping(conn_) != 0) {
            PTLog::Warn("MySQL ping fail, reconnect: %s", mysql_error(conn_));
            mysql_close(conn_);
            conn_ = mysql_init(nullptr);
            bool reconnect = true;
            mysql_options(conn_, MYSQL_OPT_RECONNECT, &reconnect);
            mysql_options(conn_, MYSQL_SET_CHARSET_NAME, "utf8mb4");
            if (!mysql_real_connect(conn_, host_.c_str(), user_.c_str(),
                                    password_.c_str(), dbname_.c_str(),
                                    port_, nullptr, 0)) {
                PTLog::Error("Reconnect fail: %s", mysql_error(conn_));
                mysql_close(conn_); conn_ = nullptr;
                return false;
            }
        }
        return true;
    }

    std::string Database::Escape(const std::string& in) {
        if (!conn_) return in;
        std::string out; out.resize(in.size() * 2 + 1);
        auto len = mysql_real_escape_string(conn_, &out[0], in.c_str(),
                                            (unsigned long)in.size());
        out.resize(len);
        return out;
    }

    bool Database::Insert(const PlayerEvent& e) {
        std::lock_guard<std::mutex> lk(mtx_);
        if (!EnsureConnection()) return false;

        const auto now = std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::system_clock::now().time_since_epoch()).count();

        std::ostringstream q;
        q << "INSERT INTO `" << table_ << "` "
          << "(`event_type`,`server_name`,`eos_id`,`player_name`,"
          << "`pos_x`,`pos_y`,`pos_z`,`created_at`) VALUES ('"
          << Escape(e.event_type)  << "','"
          << Escape(e.server_name) << "','"
          << Escape(e.eos_id)      << "','"
          << Escape(e.player_name) << "',";

        if (e.has_position) {
            q << e.x << "," << e.y << "," << e.z;
        } else {
            q << "NULL,NULL,NULL";
        }
        q << "," << now << ");";

        if (mysql_query(conn_, q.str().c_str()) != 0) {
            PTLog::Error("Insert event échoué: %s", mysql_error(conn_));
            return false;
        }
        return true;
    }
}
