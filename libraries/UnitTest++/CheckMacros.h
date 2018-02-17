#ifndef UNITTEST_CHECKMACROS_H
#define UNITTEST_CHECKMACROS_H

#include "Checks.h"
#include "MemoryOutStream.h"
#include "CurrentTest.h"
#include "TerminateTestException.h"

#ifdef CHECK
  #error UnitTest++ redefines CHECK
#endif

#ifdef CHECK_EQUAL
  #error UnitTest++ redefines CHECK_EQUAL
#endif

#ifdef CHECK_CLOSE
  #error UnitTest++ redefines CHECK_CLOSE
#endif

#ifdef CHECK_ARRAY_EQUAL
  #error UnitTest++ redefines CHECK_ARRAY_EQUAL
#endif

#ifdef CHECK_ARRAY_CLOSE
  #error UnitTest++ redefines CHECK_ARRAY_CLOSE
#endif

#ifdef CHECK_ARRAY2D_CLOSE
  #error UnitTest++ redefines CHECK_ARRAY2D_CLOSE
#endif

#ifdef _MSC_VER
#include <windows.h>
#define BREAK_UNDER_DEBUGGER (IsDebuggerPresent() && (DebugBreak(), true))
#else
#define BREAK_UNDER_DEBUGGER ((void)0)
#endif

#define CHECK_INTERNAL(condition, filename, line, id)                           \
  do                                                                            \
  {                                                                             \
      try                                                                       \
      {                                                                         \
          UnitTest::MemoryOutStream CHECK_INTERNAL_stream;                      \
          if (!(condition))                                                     \
          {                                                                     \
              BREAK_UNDER_DEBUGGER;                                             \
              UnitTest::CurrentTest::OnTestFailure((filename), (line),          \
                                                   UnitTest::ToCString(id),     \
                                                   CHECK_INTERNAL_stream.GetText()); \
          }                                                                     \
      }                                                                         \
      catch (UnitTest::TerminateTestException const&)                           \
      {                                                                         \
          throw;                                                                \
      }                                                                         \
      catch (const std::runtime_error& e)                                       \
      {                                                                         \
          BREAK_UNDER_DEBUGGER;                                                 \
          UnitTest::MemoryOutStream stream;                                     \
          stream << "Unhandled exception in check macro: " << e.what();         \
          UnitTest::CurrentTest::OnTestFailure((filename), (line),              \
                                               UnitTest::ToCString(id),         \
                                               stream.GetText());               \
      }                                                                         \
      catch (...)                                                               \
      {                                                                         \
          BREAK_UNDER_DEBUGGER;                                                 \
          UnitTest::CurrentTest::OnTestFailure((filename), (line),              \
                                               UnitTest::ToCString(id),         \
                                               "Unhandled exception in check macro"); \
      }                                                                         \
  } while (0)

#define CHECK(value) \
  CHECK_INTERNAL((CHECK_INTERNAL_stream << #value, !!(value)), __FILE__, __LINE__, NULL)

#define CHECK_SIGNED(value, id) \
  CHECK_INTERNAL((CHECK_INTERNAL_stream << #value, !!(value)), __FILE__, __LINE__, (id))

#define CHECK_EQUAL(actual, expected) \
  CHECK_INTERNAL(UnitTest::MakeActual(actual).Expected(expected).CheckEqual(CHECK_INTERNAL_stream), __FILE__, __LINE__, NULL)

#define CHECK_BIGGER(value, biggerThan) \
  CHECK_INTERNAL(UnitTest::MakeActual(value).Expected(biggerThan).CheckBigger(CHECK_INTERNAL_stream), __FILE__, __LINE__, NULL)

#define CHECK_EQUAL_SIGNED(actual, expected, id) \
  CHECK_INTERNAL(UnitTest::MakeActual(actual).Expected(expected).CheckEqual(CHECK_INTERNAL_stream), __FILE__, __LINE__, (id))

#define CHECK_CLOSE(actual, expected, tolerance) \
  CHECK_INTERNAL(UnitTest::MakeActual(actual).Expected(expected).CheckClose(tolerance, CHECK_INTERNAL_stream), __FILE__, __LINE__, NULL)

#define CHECK_CLOSE_PERCENTAGE(actual, expected, tolerancePercentage) \
  CHECK_INTERNAL(UnitTest::MakeActual(actual).Expected(expected).CheckClosePercentage(tolerancePercentage, CHECK_INTERNAL_stream), __FILE__, __LINE__, NULL)

#define CHECK_CLOSE_PERCENTAGE_SIGNED(actual, expected, tolerancePercentage, id) \
  CHECK_INTERNAL(UnitTest::MakeActual(actual).Expected(expected).CheckClosePercentage(tolerancePercentage, CHECK_INTERNAL_stream), __FILE__, __LINE__, (id))

#define CHECK_CONTAINS(actual, substring) \
  CHECK_INTERNAL(UnitTest::MakeActual(actual).Expected(substring).CheckContains(CHECK_INTERNAL_stream), __FILE__, __LINE__, NULL)

#define CHECK_STARTS_WITH(actual, prefix) \
  CHECK_INTERNAL(UnitTest::MakeActual(actual).Expected(prefix).CheckStartsWith(CHECK_INTERNAL_stream), __FILE__, __LINE__, NULL)

#define CHECK_DOES_NOT_START_WITH(actual, prefix) \
  CHECK_INTERNAL(UnitTest::MakeActual(actual).Expected(prefix).CheckDoesNotStartWith(CHECK_INTERNAL_stream), __FILE__, __LINE__, NULL)

#define CHECK_ARRAY_EQUAL(actual, expected, count) \
  CHECK_INTERNAL(UnitTest::MakeActual(actual).Expected(expected).CheckArrayEqual(count, CHECK_INTERNAL_stream), __FILE__, __LINE__, NULL)

#define CHECK_ARRAY_CLOSE(actual, expected, count, tolerance) \
  CHECK_INTERNAL(UnitTest::MakeActual(actual).Expected(expected).CheckArrayClose(count, tolerance, CHECK_INTERNAL_stream), __FILE__, __LINE__, NULL)

#define CHECK_THROW(expression, ExpectedExceptionType)                          \
  do                                                                            \
  {                                                                             \
      try                                                                       \
      {                                                                         \
          try                                                                   \
          {                                                                     \
              expression;                                                       \
          }                                                                     \
          catch (const ExpectedExceptionType&)                                  \
          {                                                                     \
              break;                                                            \
          }                                                                     \
      }                                                                         \
      catch (const std::runtime_error& e)                                           \
      {                                                                         \
          UnitTest::MemoryOutStream stream;                                     \
          stream << "Unhandled exception in check macro: " << e.what();         \
          UnitTest::CurrentTest::OnTestFailure(__FILE__, __LINE__, NULL,        \
                                               stream.GetText());               \
      }                                                                         \
      catch (...)                                                               \
      {                                                                         \
          UnitTest::CurrentTest::OnTestFailure(__FILE__, __LINE__, NULL,        \
                                               "Unhandled exception in check macro"); \
      }                                                                         \
      UnitTest::CurrentTest::OnTestFailure(__FILE__, __LINE__, NULL,            \
                                           "Expected exception " #ExpectedExceptionType " not thrown"); \
  } while (0)

#define CHECK_ASSERT(expression) \
    CHECK_THROW(expression, UnitTest::AssertException);

#endif
