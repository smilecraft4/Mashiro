#include "pch.h"

#include "Log.h"

// TODO: Consider adding timestamp with ms precision

void Log::Info(const std::wstring &msg) noexcept {
    std::wstring log = TEXT("[MASHIRO] [INFO]: ") + msg + TEXT("\n");
    OutputDebugString(log.c_str());
}

void Log::Warning(const std::wstring &msg) noexcept {
    std::wstring log = TEXT("[MASHIRO] \033[33m[WARNING]\033[0m: ") + msg + TEXT("\n");
    OutputDebugString(log.c_str());
}

void Log::Error(const std::wstring &msg) noexcept {
    std::wstring log = TEXT("[MASHIRO] \033[31m[ERROR]\033[0m: ") + msg + TEXT("\n");
    OutputDebugString(log.c_str());
}

void Log::Critical(const std::wstring &msg) noexcept {
    std::wstring log = TEXT("[MASHIRO] [CRITICAL]\033[0m: ") + msg + TEXT("\n");
    OutputDebugString(log.c_str());
    MessageBox(NULL, log.c_str(), L"Mashiro critical error", MB_ICONERROR | MB_OK);
}

void Log::Success(const std::wstring &msg) noexcept {
    std::wstring log = TEXT("[MASHIRO] \033[32m[SUCCESS]\033[0m: ") + msg + TEXT("\n");
    OutputDebugString(log.c_str());
}

void Log::Trace(const std::wstring &msg) noexcept {
    std::wstring log = TEXT("[MASHIRO] \033[37m[TRACE]\033[0m: ") + msg + TEXT("\n");
    OutputDebugString(log.c_str());
}

void Log::Debug(const std::wstring &msg) noexcept {
    std::wstring log = TEXT("[MASHIRO] \033[36m[DEBUG]\033[0m: ") + msg + TEXT("\n");
    OutputDebugString(log.c_str());
}
