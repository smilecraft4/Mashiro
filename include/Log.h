#pragma once
#include "Framework.h"

class Log {
public:
	Log(const Log&) = delete;
	Log(Log&&) = delete;
	Log& operator=(const Log&) = delete;
	Log& operator=(Log&&) = delete;

	static void Info(const tstring& msg) noexcept;
	static void Trace(const tstring& msg) noexcept;
};
