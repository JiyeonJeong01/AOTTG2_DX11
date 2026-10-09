#pragma once

#include "Logger.h"

#if defined(_MSC_VER)
#include <intrin.h>
#define DEBUG_BREAK() __debugbreak()
#else
#include <signal.h>
#define DEBUG_BREAK() raise(SIGTRAP)
#endif

#define EXPAND_MACRO(x) x
#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(EXPAND_MACRO(x))
#define E_TO_STR(E) STRINGIFY(E)
 
/* --- Log macros --- */
/* 매크로는 호출한 TU에서 전개되므로, Engine 프로젝트(ENGINE_EXPORTS)에서 컴파일되면 ENGINE으로 찍힌다.
 * Engine 헤더의 inline/template 함수는 include한 쪽 TU에서 전개되므로 그쪽 도메인을 따른다. */
#ifdef ENGINE_EXPORTS
#define LOG_DOMAIN DOMAIN_TYPE::ENGINE
#else
#define LOG_DOMAIN DOMAIN_TYPE::CLIENT
#endif

#define LOG_IMPL(sev, dom, fmt, ...) \
  do { SYS_LOG.Log(sev, dom, __FILE__, __FUNCTION__, nullptr, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#define LOG_INFO(fmt, ...)  LOG_IMPL(SEVERITY_TYPE::INFO,  LOG_DOMAIN, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  LOG_IMPL(SEVERITY_TYPE::WARN,  LOG_DOMAIN, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG_IMPL(SEVERITY_TYPE::ERR,   LOG_DOMAIN, fmt, ##__VA_ARGS__)

#define DEBUG_POINT         int a = 10;

#define ERROR_BREAK(fmt, ...) \
  do { \
    LOG_ERROR(fmt, ##__VA_ARGS__); \
    DEBUG_BREAK(); \
  } while(0)

#define LOG_INFO_BREAK(fmt, ...) \
  do { \
    LOG_INFO(fmt, ##__VA_ARGS__); \
    DEBUG_BREAK(); \
  } while(0)

#define NULL_BREAK_RETURN_MSG(_ptr, _return, fmt, ...)          \
    do {                                                        \
        if (!(_ptr)) {                                          \
            ERROR_BREAK(fmt, ##__VA_ARGS__);                    \
            return _return;                                     \
        }                                                       \
    } while (0)

#define FAIL_BREAK_RETURN_MSG(_res, _return, fmt, ...)          \
    do {                                                        \
        if (FAILED(_res)) {                                     \
            ERROR_BREAK(fmt, ##__VA_ARGS__);                    \
            return _return;                                     \
        }                                                       \
    } while (0)

#define TRUE_BREAK_RETURN_MSG(_check, _return, fmt, ...)        \
    do {                                                        \
        if ((_check)) {                                         \
            ERROR_BREAK(fmt, ##__VA_ARGS__);                    \
            return _return;                                     \
        }                                                       \
    } while (0)


/* Execute only in debug mode */
#ifdef _DEBUG
#define ENABLE_LOGS
#endif

#ifdef ENABLE_LOGS
#define _DEBUG_ERROR_BREAK(fmt, ...)    ERROR_BREAK(fmt, ##__VA_ARGS__)
#define _DEBUG_INFO_BREAK(fmt, ...)     LOG_INFO_BREAK(fmt, ##__VA_ARGS__)
#define _DEBUG_ERROR(fmt, ...)          LOG_ERROR(fmt, ##__VA_ARGS__)
#define _DEBUG_WARN(fmt, ...)           LOG_WARN(fmt, ##__VA_ARGS__)
#define _DEBUG_INFO(fmt, ...)           LOG_INFO(fmt, ##__VA_ARGS__)
#define IF_NULL_RETURN_MSG_BREAK(_ptr, _return, fmt, ...)       NULL_BREAK_RETURN_MSG(_ptr, _return, fmt, ##__VA_ARGS__)
#define IF_FAIL_RETURN_MSG_BREAK(_res, _return, fmt, ...)       FAIL_BREAK_RETURN_MSG(_res, _return, fmt, ##__VA_ARGS__)
#define IF_TRUE_RETURN_MSG_BREAK(_check, _return, fmt, ...)     TRUE_BREAK_RETURN_MSG(_check, _return, fmt, ##__VA_ARGS__)
#else
#define _DEBUG_ERROR_BREAK(fmt, ...)    ((void)0)
#define _DEBUG_INFO_BREAK(fmt, ...)     ((void)0)
#define _DEBUG_ERROR(fmt, ...)          ((void)0)
#define _DEBUG_WARN(fmt, ...)           ((void)0)
#define _DEBUG_INFO(fmt, ...)           ((void)0)
#define IF_NULL_RETURN_MSG_BREAK(_ptr, _return, fmt, ...) \
        do { if (nullptr == (_ptr)) return _return; } while(0)

#define IF_FAIL_RETURN_MSG_BREAK(_res, _return, fmt, ...) \
        do { if (FAILED(_res)) return _return; } while(0)

#define IF_TRUE_RETURN_MSG_BREAK(_check, _return, fmt, ...) \
        do { if ((_check)) return _return; } while(0)
#endif

/* --- Assert macros --- */
#ifdef _DEBUG
#define ENABLE_ASSERTS
#endif

#ifdef ENABLE_ASSERTS

#define INTERNAL_ASSERT_IMPL(domain, check, fmt, ...) \
    do { \
      if (!(check)) { \
        SYS_LOG.Assert(domain, __FILE__, __FUNCTION__, STRINGIFY(check), __LINE__, (fmt) ? (fmt) : "", ##__VA_ARGS__); \
        DEBUG_BREAK(); \
      } \
    } while (0)

#define INTERNAL_ASSERT_WITH_MSG(domain, check, fmt, ...) \
    INTERNAL_ASSERT_IMPL(domain, check, fmt, ##__VA_ARGS__)

#define INTERNAL_ASSERT_NO_MSG(domain, check) \
    INTERNAL_ASSERT_IMPL(domain, check, "")

#define _DEBUG_ASSERT(check) \
  INTERNAL_ASSERT_NO_MSG(LOG_DOMAIN, check)

#define _DEBUG_ASSERT_MSG(check, fmt, ...) \
  INTERNAL_ASSERT_WITH_MSG(LOG_DOMAIN, check, fmt, ##__VA_ARGS__)

#define _DEBUG_ENGINE_ASSERT(check) \
  INTERNAL_ASSERT_NO_MSG(DOMAIN_TYPE::ENGINE, check)

#define _DEBUG_ENGINE_ASSERT_MSG(check, fmt, ...) \
  INTERNAL_ASSERT_WITH_MSG(DOMAIN_TYPE::ENGINE, check, fmt, ##__VA_ARGS__)

#else
#define _DEBUG_ASSERT(...)                         ((void)0)
#define _DEBUG_ASSERT_MSG(check, fmt, ...)         ((void)0)
#define _DEBUG_ENGINE_ASSERT(...)                  ((void)0)
#define _DEBUG_ENGINE_ASSERT_MSG(check, fmt, ...)  ((void)0)
#endif



