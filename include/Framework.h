#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>
#include <comdef.h>
#include <commctrl.h>
#include <string>
#include <glad/glad.h>

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>

#include "wintab.h"

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Opengl32.lib")

#ifdef UNICODE
using tstring = std::wstring;
#else
using tstring = std::string;
#endif


#include "msgpack.h"
#include "wintab.h"
#define PACKETDATA	(PK_X | PK_Y | PK_BUTTONS | PK_NORMAL_PRESSURE | PK_TANGENT_PRESSURE | PK_ORIENTATION | PK_TIME )
#define PACKETMODE	PK_BUTTONS
#include "pktdef.h"

#define MAX_PACKETS	20

#ifdef UNICODE 
#define ConvertString ConvertStringW
#define RestoreStringA ConvertStringA
#define RestoreStringW ((void)0)
#else 
#define ConvertString ConvertStringA
#define RestoreStringA ((void)0)
#define RestoreStringW ConvertStringW
#endif

std::wstring ConvertStringW(const std::string& string);
std::string ConvertStringA(const std::wstring& wstring);

void HrCheck(HRESULT result, int line, const char* function, const char* file);

#define HR_CHECK(x) HrCheck(x, __LINE__, __FUNCTION__, __FILE__)
#define WIN32_CHECK(x) HR_CHECK(HRESULT_FROM_WIN32(x))

/*----------------------------------------------------------------------------s
	NAME
		Utils.h

	PURPOSE
		Defines for the general-purpose functions for the WinTab demos.

	COPYRIGHT
		This file is Copyright (c) Wacom Company, Ltd. 2024 All Rights Reserved
		with portions copyright 1991-1998 by LCS/Telegraphics.

		The text and information contained in this file may be freely used,
		copied, or distributed without compensation or licensing restrictions.
---------------------------------------------------------------------------- */

//////////////////////////////////////////////////////////////////////////////
#define WACOM_DEBUG

// Ignore warnings about using unsafe string functions.
#pragma warning( disable : 4996 )

//////////////////////////////////////////////////////////////////////////////
// Function pointers to Wintab functions exported from wintab32.dll. 
typedef UINT(API* WTINFOA) (UINT, UINT, LPVOID);
typedef HCTX(API* WTOPENA)(HWND, LPLOGCONTEXTA, bool);
typedef HCTX(API* WTOPENW)(HWND, LPLOGCONTEXTW, bool);
typedef bool (API* WTGETA) (HCTX, LPLOGCONTEXT);
typedef bool (API* WTSETA) (HCTX, LPLOGCONTEXT);
typedef bool (API* WTCLOSE) (HCTX);
typedef bool (API* WTENABLE) (HCTX, bool);
typedef bool (API* WTPACKET) (HCTX, UINT, LPVOID);
typedef bool (API* WTOVERLAP) (HCTX, bool);
typedef bool (API* WTSAVE) (HCTX, LPVOID);
typedef bool (API* WTCONFIG) (HCTX, HWND);
typedef HCTX(API* WTRESTORE) (HWND, LPVOID, bool);
typedef bool (API* WTEXTSET) (HCTX, UINT, LPVOID);
typedef bool (API* WTEXTGET) (HCTX, UINT, LPVOID);
typedef bool (API* WTQUEUESIZESET) (HCTX, int);
typedef int  (API* WTDATAPEEK) (HCTX, UINT, UINT, int, LPVOID, LPINT);
typedef int  (API* WTPACKETSGET) (HCTX, int, LPVOID);
typedef HMGR(API* WTMGROPEN) (HWND, UINT);
typedef bool (API* WTMGRCLOSE) (HMGR);
typedef HCTX(API* WTMGRDEFCONTEXT) (HMGR, bool);
typedef HCTX(API* WTMGRDEFCONTEXTEX) (HMGR, UINT, bool);

// TODO - add more wintab32 function defs as needed

//////////////////////////////////////////////////////////////////////////////
extern char* gpszProgramName;

// Loaded Wintab32 API functions.
extern HINSTANCE ghWintab;

extern WTINFOA gpWTInfoA;
extern WTOPENA gpWTOpenA;
extern WTOPENW gpWTOpenW;
extern WTGETA gpWTGetA;
extern WTSETA gpWTSetA;
extern WTCLOSE gpWTClose;
extern WTPACKET gpWTPacket;
extern WTENABLE gpWTEnable;
extern WTOVERLAP gpWTOverlap;
extern WTSAVE gpWTSave;
extern WTCONFIG gpWTConfig;
extern WTRESTORE gpWTRestore;
extern WTEXTSET gpWTExtSet;
extern WTEXTGET gpWTExtGet;
extern WTQUEUESIZESET gpWTQueueSizeSet;
extern WTDATAPEEK gpWTDataPeek;
extern WTPACKETSGET gpWTPacketsGet;
extern WTMGROPEN gpWTMgrOpen;
extern WTMGRCLOSE gpWTMgrClose;
extern WTMGRDEFCONTEXT gpWTMgrDefContext;
extern WTMGRDEFCONTEXTEX gpWTMgrDefContextEx;

// TODO - add more wintab32 function pointers as needed

//////////////////////////////////////////////////////////////////////////////
bool LoadWintab(void);
void UnloadWintab(void);

void ShowError(const char* pszErrorMessage);

//////////////////////////////////////////////////////////////////////////////
#ifdef WACOM_DEBUG

void WacomTrace(const char* lpszFormat, ...);

#define WACOM_ASSERT( x ) assert( x )
#define WACOM_TRACE(...)  WacomTrace(__VA_ARGS__)
#else
#define WACOM_TRACE(...)
#define WACOM_ASSERT( x )

#endif // WACOM_DEBUG


