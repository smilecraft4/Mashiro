#pragma once

// TODO: Normaly this file should not be modified so maybe using this as a precompiled header would be wise as some of
// the heavy headers are here (Windows.h)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <comdef.h>
#include <commctrl.h>
#include <glad/glad.h>
#include <string>
#include <windowsx.h>

#include <assert.h>
#include <stdarg.h>
#include <stdio.h>

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Opengl32.lib")

// TODO: Do i really need support ANSI, changing everything std::wstring would simplify a lot the whole application,
// The problem is regarding the exceptions as they are std::string by default and I don't know how to change it to
// std::wstring

#ifdef UNICODE
using tstring = std::wstring;
#else
using tstring = std::string;
#endif

#ifdef UNICODE
#define ConvertString ConvertStringW
#define RestoreStringA ConvertStringA
#define RestoreStringW ((void)0)
#else
#define ConvertString ConvertStringA
#define RestoreStringA ((void)0)
#define RestoreStringW ConvertStringW
#endif

std::wstring ConvertStringW(const std::string &string);
std::string ConvertStringA(const std::wstring &wstring);

// TODO: is this really needed
void HrCheck(HRESULT result, int line, const char *function, const char *file);

#define HR_CHECK(x) HrCheck(x, __LINE__, __FUNCTION__, __FILE__)
#define WIN32_CHECK(x) HR_CHECK(HRESULT_FROM_WIN32(x))