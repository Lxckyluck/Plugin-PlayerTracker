#pragma once

#include <cstdio>
#include <cstdarg>
#include <string>

namespace PTLog {

    inline std::string Format(const char* fmt, va_list args) {
        char buf[1024];
        vsnprintf(buf, sizeof(buf), fmt, args);
        return std::string(buf);
    }

    inline void Info(const char* fmt, ...) {
        va_list args; va_start(args, fmt);
        std::string msg = Format(fmt, args);
        va_end(args);
        std::printf("[PlayerTracker][INFO ] %s\n", msg.c_str());
    }
    inline void Warn(const char* fmt, ...) {
        va_list args; va_start(args, fmt);
        std::string msg = Format(fmt, args);
        va_end(args);
        std::printf("[PlayerTracker][WARN ] %s\n", msg.c_str());
    }
    inline void Error(const char* fmt, ...) {
        va_list args; va_start(args, fmt);
        std::string msg = Format(fmt, args);
        va_end(args);
        std::fprintf(stderr, "[PlayerTracker][ERROR] %s\n", msg.c_str());
    }
}
