#include "Framework.h"
#include "Log.h"

#include <format>

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

void HrCheck(HRESULT result, int line, const char *function, const char *file) {
    if (FAILED(result)) {
        // Format result
        _com_error err(result);

        std::string text = std::format("{}\n{}: {}\n\n{}", file, function, line, ConvertStringA(err.ErrorMessage()));
        MessageBox(nullptr, ConvertString(text).c_str(), TEXT("Mashiro"), MB_ICONERROR | MB_OK);
    }
}
