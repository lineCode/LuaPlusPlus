#include <StringUtil.hpp>
#include <cstdarg>
#include <array>
#include <memory>
#include <cassert>

#ifdef WIN32
#define FORMAT_STRING(string) __format_string string
#else
#define FORMAT_STRING(string) string
#endif

namespace StringUtil
{
  constexpr int ssprintf_buffer_size = 512;
  thread_local std::array<char, ssprintf_buffer_size> ssprintf_buffer;

  std::string ssprintf(FORMAT_STRING(const char* format), ...)
  {
    va_list args;
    va_start(args, format);

    std::string result;

    auto written = vsnprintf(ssprintf_buffer.data(), ssprintf_buffer_size, format, args);
    va_end(args);

    if (written >= (int)ssprintf_buffer_size)
    {
      va_list args2;
      va_start(args2, format);
      auto large_buf = std::make_unique<char[]>(written+1);
      auto written2 = vsnprintf(large_buf.get(), written+1, format, args2);
      assert(written2 == written);
      result = large_buf.get();
      va_end(args2);
    }
    else
      result = ssprintf_buffer.data();

    return result;
  }

  std::string vssprintf(const char* format, va_list args)
  {
    std::string result;
    va_list args2;
    va_copy(args2, args);

    auto written = vsnprintf(ssprintf_buffer.data(), ssprintf_buffer_size, format, args);
    if (written >= (int)ssprintf_buffer_size)
    {
      auto large_buf = std::make_unique<char[]>(written);
      auto written2 = vsnprintf(large_buf.get(), written, format, args2);
      assert(written2 == written);
      result = large_buf.get();
    }
    else
      result = ssprintf_buffer.data();

    va_end(args2);

    return result;
  }

  std::vector<std::string> split(const std::string& delimiter, const std::string& input)
  {
    std::vector<std::string> result;
    if (input.empty())
      return result;

    size_t startOfNextWord = 0;
    size_t spacePosition = input.find_first_of(delimiter);
    while (true)
    {
      if (spacePosition == std::string::npos)
      {
        result.push_back(input.substr(startOfNextWord, input.size() - startOfNextWord));
        return result;
      }

      result.push_back(input.substr(startOfNextWord, spacePosition - startOfNextWord));
      startOfNextWord = spacePosition + 1;
      spacePosition = input.find_first_of(delimiter, startOfNextWord);
    }
  }
}

#undef FORMAT_STRING
