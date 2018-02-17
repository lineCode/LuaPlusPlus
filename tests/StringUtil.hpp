#pragma once
#include <string>
#include <vector>

#ifdef LINUX // Clang uses different attributes for (u)int64_t - ll vs l
#define FORMAT_ATTR __attribute__((format(printf, 1, 2)))
#else
#define FORMAT_ATTR
#endif

#ifdef WIN32
#define FORMAT_STRING(string) __format_string string
#else
#define FORMAT_STRING(string) string
#endif

namespace StringUtil
{
  std::string ssprintf(FORMAT_STRING(const char* format), ...) FORMAT_ATTR;
  std::string vssprintf(const char* format, va_list args);
  std::vector<std::string> split(const std::string& delimiter, const std::string& input);
}

#undef FORMAT_STRING
#undef FORMAT_ATTR
