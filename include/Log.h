#pragma once
#include "Framework.h"

// TODO: Add macro for better trace optimization, is constexpr better

#ifndef NDEBUG
// #define MS_TRACE
// #define MS_DEBUG
#define MS_INFO
#define MS_WARNING
#define MS_ERROR
#define MS_CRITICAL
#define MS_SUCCESS
#else
// #define MS_TRACE
// #define MS_DEBUG
// #define MS_INFO
#define MS_WARNING
#define MS_ERROR
#define MS_CRITICAL
// #define MS_SUCCESS
#endif // DEBUG

class Log {
  public:
    Log(const Log &) = delete;
    Log(Log &&) = delete;
    Log &operator=(const Log &) = delete;
    Log &operator=(Log &&) = delete;

    // TODO: Maybe coloring on this
    static void Trace(const tstring &msg) noexcept;
    static void Debug(const tstring &msg) noexcept;
    static void Info(const tstring &msg) noexcept;
    static void Warning(const tstring &msg) noexcept;
    static void Error(const tstring &msg) noexcept;
    static void Critical(const tstring &msg) noexcept;
    static void Success(const tstring &msg) noexcept;
};

#ifdef MS_TRACE
#define LOG_TRACE(x) Log::Trace(x)
#else
#define LOG_TRACE(x) (void)0
#endif

#ifdef MS_DEBUG
#define LOG_DEBUG(x) Log::Debug(x)
#else
#define LOG_DEBUG(x) (void)0
#endif

#ifdef MS_INFO
#define LOG_INFO(x) Log::Info(x)
#else
#define LOG_INFO(x) (void)0
#endif

#ifdef MS_WARNING
#define LOG_WARNING(x) Log::Warning(x)
#else
#define LOG_WARNING(x) (void)0
#endif

#ifdef MS_ERROR
#define LOG_ERROR(x) Log::Error(x)
#else
#define LOG_ERROR(x) (void)0
#endif

#ifdef MS_CRITICAL
#define LOG_CRITICAL(x) Log::Critical(x)
#else
#define LOG_CRITICAL(x) (void)0
#endif

#ifdef MS_SUCCESS
#define LOG_SUCCESS(x) Log::Success(x)
#else
#define LOG_SUCCESS(x) (void)0
#endif
