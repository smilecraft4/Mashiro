#include "Log.h"

void Log::Info(const tstring& msg) noexcept
{
	tstring log = TEXT("[MASHIRO] [INFO]: ") + msg + TEXT("\n");
	OutputDebugString(log.c_str());
}

void Log::Trace(const tstring& msg) noexcept {
	tstring log = TEXT("[MASHIRO] [TRACE]: ") + msg + TEXT("\n");
	OutputDebugString(log.c_str());
}
