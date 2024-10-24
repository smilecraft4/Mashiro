#include "Framework.h"
#include "Log.h"

#include <format>

std::wstring ConvertStringW(const std::string& string) {
	const int count = MultiByteToWideChar(CP_UTF8, 0, string.c_str(), static_cast<int>(string.length()), nullptr, 0);
	std::wstring wstring(count, 0);
	MultiByteToWideChar(CP_UTF8, 0, string.c_str(), -1, wstring.data(), count);
	return wstring;
}

std::string ConvertStringA(const std::wstring& wstring) {
	const int count = WideCharToMultiByte(CP_ACP, 0, wstring.c_str(), static_cast<int>(wstring.length()), nullptr, 0, nullptr, nullptr);
	std::string string(count, 0);
	WideCharToMultiByte(CP_ACP, 0, wstring.c_str(), -1, string.data(), count, nullptr, nullptr);
	return string;
}

void HrCheck(HRESULT result, int line, const char* function, const char* file) {
	if (FAILED(result)) {
		// Format result
		_com_error err(result);

		std::string text = std::format("{}\n{}: {}\n\n{}", file, function, line, ConvertStringA(err.ErrorMessage()));
		MessageBox(nullptr, ConvertString(text).c_str(), TEXT("Mashiro"), MB_ICONERROR | MB_OK);
	}
}

/*----------------------------------------------------------------------------
	NAME
		Utils.c

	PURPOSE
		Defines for the general-purpose functions for the WinTab demos.

	COPYRIGHT
		This file is Copyright (c) Wacom Company, Ltd. 2024 All Rights Reserved
		with portions copyright 1991-1998 by LCS/Telegraphics.

		The text and information contained in this file may be freely used,
		copied, or distributed without compensation or licensing restrictions.
---------------------------------------------------------------------------- */

//////////////////////////////////////////////////////////////////////////////
HINSTANCE ghWintab = NULL;

WTINFOA gpWTInfoA = NULL;
WTOPENA gpWTOpenA = NULL;
WTOPENW gpWTOpenW = NULL;
WTGETA gpWTGetA = NULL;
WTSETA gpWTSetA = NULL;
WTCLOSE gpWTClose = NULL;
WTPACKET gpWTPacket = NULL;
WTENABLE gpWTEnable = NULL;
WTOVERLAP gpWTOverlap = NULL;
WTSAVE gpWTSave = NULL;
WTCONFIG gpWTConfig = NULL;
WTRESTORE gpWTRestore = NULL;
WTEXTSET gpWTExtSet = NULL;
WTEXTGET gpWTExtGet = NULL;
WTQUEUESIZESET gpWTQueueSizeSet = NULL;
WTDATAPEEK gpWTDataPeek = NULL;
WTPACKETSGET gpWTPacketsGet = NULL;
WTMGROPEN gpWTMgrOpen = NULL;
WTMGRCLOSE gpWTMgrClose = NULL;
WTMGRDEFCONTEXT gpWTMgrDefContext = NULL;
WTMGRDEFCONTEXTEX gpWTMgrDefContextEx = NULL;

char* pszProgramName = NULL;

// GETPROCADDRESS macro used to create the gpWT* () dynamic function pointers, which allow the
// same built program to run on both 32bit and 64bit systems w/o having to rebuild the app.
#define GETPROCADDRESS(type, func) \
	gp##func = (type)GetProcAddress(ghWintab, #func); \
	if (!gp##func){ assert(false); UnloadWintab(); return false; }

//////////////////////////////////////////////////////////////////////////////
// Purpose
//		Find wintab32.dll and load it.  
//		Find the exported functions we need from it.
//
//	Returns
//		true on success.
//		false on failure.
//
bool LoadWintab(void) {
	// Wintab32.dll is a module installed by the tablet driver
	ghWintab = LoadLibraryA("Wintab32.dll");

	if (!ghWintab) {
		DWORD err = GetLastError();
		Log::Info(std::format(TEXT("LoadLibrary error: %i\n"), err));
		throw std::runtime_error("Could not load Wintab32.dll");
		return false;
	}

	// Explicitly find the exported Wintab functions in which we are interested.
	// We are using the ASCII, not unicode versions (where applicable).
	GETPROCADDRESS(WTOPENA, WTOpenA);
	GETPROCADDRESS(WTOPENW, WTOpenW);
	GETPROCADDRESS(WTINFOA, WTInfoA);
	GETPROCADDRESS(WTGETA, WTGetA);
	GETPROCADDRESS(WTSETA, WTSetA);
	GETPROCADDRESS(WTPACKET, WTPacket);
	GETPROCADDRESS(WTCLOSE, WTClose);
	GETPROCADDRESS(WTENABLE, WTEnable);
	GETPROCADDRESS(WTOVERLAP, WTOverlap);
	GETPROCADDRESS(WTSAVE, WTSave);
	GETPROCADDRESS(WTCONFIG, WTConfig);
	GETPROCADDRESS(WTRESTORE, WTRestore);
	GETPROCADDRESS(WTEXTSET, WTExtSet);
	GETPROCADDRESS(WTEXTGET, WTExtGet);
	GETPROCADDRESS(WTQUEUESIZESET, WTQueueSizeSet);
	GETPROCADDRESS(WTDATAPEEK, WTDataPeek);
	GETPROCADDRESS(WTPACKETSGET, WTPacketsGet);
	GETPROCADDRESS(WTMGROPEN, WTMgrOpen);
	GETPROCADDRESS(WTMGRCLOSE, WTMgrClose);
	GETPROCADDRESS(WTMGRDEFCONTEXT, WTMgrDefContext);
	GETPROCADDRESS(WTMGRDEFCONTEXTEX, WTMgrDefContextEx);

	return true;
}



//////////////////////////////////////////////////////////////////////////////
// Purpose
//		Uninitializes use of wintab32.dll
//		Frees the loaded Wintab32.dll module and zeros all dynamic function pointers.
//
// Returns
//		Nothing.
//
void UnloadWintab(void) {
	Log::Info(TEXT("UnloadWintab()\n"));

	if (ghWintab) {
		FreeLibrary(ghWintab);
		ghWintab = NULL;
	}

	gpWTOpenA = NULL;
	gpWTOpenW = NULL;
	gpWTClose = NULL;
	gpWTInfoA = NULL;
	gpWTPacket = NULL;
	gpWTEnable = NULL;
	gpWTOverlap = NULL;
	gpWTSave = NULL;
	gpWTConfig = NULL;
	gpWTGetA = NULL;
	gpWTSetA = NULL;
	gpWTRestore = NULL;
	gpWTExtSet = NULL;
	gpWTExtGet = NULL;
	gpWTQueueSizeSet = NULL;
	gpWTDataPeek = NULL;
	gpWTPacketsGet = NULL;
	gpWTMgrOpen = NULL;
	gpWTMgrClose = NULL;
	gpWTMgrDefContext = NULL;
	gpWTMgrDefContextEx = NULL;
}


void ShowError(const char* pszErrorMessage) {
	assert("ShowError()\n");

	assert(pszErrorMessage);

	MessageBoxA(NULL, pszErrorMessage, "Mashiro", MB_OK | MB_ICONHAND);
}

#ifdef WACOM_DEBUG

//////////////////////////////////////////////////////////////////////////////
void WacomTrace(const char* lpszFormat, ...) {
	char szTraceMessage[128];

	int nBytesWritten;

	va_list args;

	assert(lpszFormat);

	va_start(args, lpszFormat);

	nBytesWritten = _vsnprintf(szTraceMessage, sizeof(szTraceMessage) - 1,
		lpszFormat, args);

	if (nBytesWritten > 0) {
		char szHeader[128];
		sprintf(szHeader, "[%s]: ", "Mashiro");
		OutputDebugStringA(szHeader);
		OutputDebugStringA(szTraceMessage);
	}

	va_end(args);
}

#endif // WACOM_DEBUG
