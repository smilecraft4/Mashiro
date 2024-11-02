#include "pch.h"

#include "Utils.h"

std::wstring ConvertStringW(const std::string &string) {
    const int count = MultiByteToWideChar(CP_UTF8, 0, string.c_str(), static_cast<int>(string.length()), nullptr, 0);
    std::wstring wstring(count, 0);
    MultiByteToWideChar(CP_UTF8, 0, string.c_str(), -1, wstring.data(), count);
    return wstring;
}

std::string ConvertStringA(const std::wstring &wstring) {
    const int count = WideCharToMultiByte(CP_ACP, 0, wstring.c_str(), static_cast<int>(wstring.length()), nullptr, 0,
                                          nullptr, nullptr);
    std::string string(count, 0);
    WideCharToMultiByte(CP_ACP, 0, wstring.c_str(), -1, string.data(), count, nullptr, nullptr);
    return string;
}