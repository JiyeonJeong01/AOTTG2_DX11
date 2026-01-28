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

// log macros
#define LOG_IMPL(sev, dom, fmt, ...) \
  do { LOGGER->Log(sev, dom, __FILE__, __FUNCTION__, nullptr, __LINE__, fmt, ##__VA_ARGS__); } while(0)

#define LOG_INFO(fmt, ...)  LOG_IMPL(SEVERITY_TYPE::INFO,  DOMAIN_TYPE::CLIENT, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  LOG_IMPL(SEVERITY_TYPE::WARN,  DOMAIN_TYPE::CLIENT, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) LOG_IMPL(SEVERITY_TYPE::ERR,   DOMAIN_TYPE::CLIENT, fmt, ##__VA_ARGS__)

#define ENGINE_LOG_INFO(fmt, ...)  LOG_IMPL(SEVERITY_TYPE::INFO,  DOMAIN_TYPE::ENGINE, fmt, ##__VA_ARGS__)
#define ENGINE_LOG_WARN(fmt, ...)  LOG_IMPL(SEVERITY_TYPE::WARN,  DOMAIN_TYPE::ENGINE, fmt, ##__VA_ARGS__)
#define ENGINE_LOG_ERROR(fmt, ...) LOG_IMPL(SEVERITY_TYPE::ERR,   DOMAIN_TYPE::ENGINE, fmt, ##__VA_ARGS__)

#ifdef _DEBUG
#define ENABLE_ASSERTS
#endif

#ifdef ENABLE_ASSERTS

#define INTERNAL_ASSERT_IMPL(domain, check, fmt, ...) \
    do { \
      if (!(check)) { \
        LOGGER->Assert(domain, __FILE__, __FUNCTION__, STRINGIFY(check), __LINE__, (fmt) ? (fmt) : "", ##__VA_ARGS__); \
        DEBUG_BREAK(); \
      } \
    } while (0)

#define INTERNAL_ASSERT_WITH_MSG(domain, check, fmt, ...) \
    INTERNAL_ASSERT_IMPL(domain, check, fmt, ##__VA_ARGS__)

#define INTERNAL_ASSERT_NO_MSG(domain, check) \
    INTERNAL_ASSERT_IMPL(domain, check, "")

#define ASSERT(check) \
  INTERNAL_ASSERT_NO_MSG(DOMAIN_TYPE::CLIENT, check)

#define ASSERT_MSG(check, fmt, ...) \
  INTERNAL_ASSERT_WITH_MSG(DOMAIN_TYPE::CLIENT, check, fmt, ##__VA_ARGS__)

#define ENGINE_ASSERT(check) \
  INTERNAL_ASSERT_NO_MSG(DOMAIN_TYPE::ENGINE, check)

#define ENGINE_ASSERT_MSG(check, fmt, ...) \
  INTERNAL_ASSERT_WITH_MSG(DOMAIN_TYPE::ENGINE, check, fmt, ##__VA_ARGS__)


#else
#define ASSERT(...)        ((void)0)
#define ENGINE_ASSERT(...) ((void)0)
#endif

#define WARN_IF(cond, fmt, ...) \
  do { if ((cond)) { LOG_WARN(fmt, ##__VA_ARGS__); } } while(0)

#define ERROR_IF(cond, fmt, ...) \
  do { if ((cond)) { LOG_ERROR(fmt, ##__VA_ARGS__); } } while(0)

#define ERROR_BREAK(fmt, ...) \
  do { \
    LOG_ERROR(fmt, ##__VA_ARGS__); \
    DEBUG_BREAK(); \
  } while(0)

// 디버그 모드일 때만 실행 
#ifdef _DEBUG
#define _DEBUG_ERROR_BREAK(fmt, ...) ERROR_BREAK(fmt, ##__VA_ARGS__) 
#else
#define _DEBUG_ERROR_BREAK(fmt, ...) ((void)0)
#endif

#pragma region LOG_MODE
#ifdef _DEBUG

#ifndef LOG_WITH_CONSOLE
#define LOG_WITH_CONSOLE
#endif 

//#ifndef LOG_WITH_GUI
//#define LOG_WITH_GUI
//#endif 

#endif
#pragma endregion
