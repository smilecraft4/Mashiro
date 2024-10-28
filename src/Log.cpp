#include "Log.h"

// TODO: Consider adding timestamp with ms precision

void Log::Info(const tstring &msg) noexcept {
    tstring log = TEXT("[MASHIRO] [INFO]: ") + msg + TEXT("\n");
    OutputDebugString(log.c_str());
}

void Log::Warning(const tstring &msg) noexcept {
    tstring log = TEXT("[MASHIRO] \033[33m[WARNING]\033[0m: ") + msg + TEXT("\n");
    OutputDebugString(log.c_str());
}

void Log::Error(const tstring &msg) noexcept {
    tstring log = TEXT("[MASHIRO] \033[31m[ERROR]\033[0m: ") + msg + TEXT("\n");
    OutputDebugString(log.c_str());
}

void Log::Critical(const tstring &msg) noexcept {
    tstring log = TEXT("[MASHIRO] [CRITICAL]\033[0m: ") + msg + TEXT("\n");
    OutputDebugString(log.c_str());
}

void Log::Success(const tstring &msg) noexcept {
    tstring log = TEXT("[MASHIRO] \033[32m[SUCCESS]\033[0m: ") + msg + TEXT("\n");
    OutputDebugString(log.c_str());
}

void Log::Trace(const tstring &msg) noexcept {
    tstring log = TEXT("[MASHIRO] \033[37m[TRACE]\033[0m: ") + msg + TEXT("\n");
    OutputDebugString(log.c_str());
}

void Log::Debug(const tstring &msg) noexcept {
    tstring log = TEXT("[MASHIRO] \033[36m[DEBUG]\033[0m: ") + msg + TEXT("\n");
    OutputDebugString(log.c_str());
}
