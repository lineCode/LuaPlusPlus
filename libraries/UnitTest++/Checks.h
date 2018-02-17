#pragma once
#include <Config.h>
#include <TestResults.h>
#include <MemoryOutStream.h>

namespace UnitTest
{
  const char* ToCString(const char* str);
  const char* ToCString(const std::string& str);

  template<typename Actual, typename Expected, typename Tolerance>
  bool AreClose(Actual const& actual, Expected const& expected, Tolerance const& tolerance)
  {
    return (actual >= (expected - tolerance)) && (actual <= (expected + tolerance));
  }

  template<typename Actual, typename Expected>
  bool CheckEqual(Actual const& actual, Expected const& expected, MemoryOutStream& stream)
  {
    if (actual == expected)
      return true;
    stream << "Expected " << expected << " but was " << actual;
    return false;
  }

  template<typename Actual, typename Expected>
  bool CheckBigger(Actual const& value, Expected const& biggerThan, MemoryOutStream& stream)
  {
    if (value > biggerThan)
      return true;

    stream << "Value " << value << " wasn't bigger than " << biggerThan;
    return false;
  }

  bool CheckEqual(char const* actual, char const* expected, MemoryOutStream& stream);

  template<typename Actual, typename Expected, typename Tolerance>
  bool CheckClose(Actual const& actual, Expected const& expected, Tolerance const& tolerance,
                  MemoryOutStream& stream)
  {
    if (AreClose(actual, expected, tolerance))
      return true;

    stream << "Expected " << expected << " +/- " << tolerance << " but was " << actual;
    return false;
  }

  template<typename Actual, typename Expected, typename Tolerance>
  bool CheckClosePercentage(Actual const& actual, Expected const& expected, Tolerance const& tolerancePercentage,
                            MemoryOutStream& stream)
  {
    Expected toleranceAbsolute = tolerancePercentage * expected / 100;
    if (toleranceAbsolute < 0)
      toleranceAbsolute = -toleranceAbsolute;

    if (AreClose(actual, expected, toleranceAbsolute))
      return true;

    stream << "Expected " << expected << " +/- " << tolerancePercentage << "% (" << toleranceAbsolute << ") but was " << actual;
    return false;
  }

  bool CheckContains(const char* actual, const char* substring, MemoryOutStream& stream);
  bool CheckContains(const std::string& actual, const char* substring, MemoryOutStream& stream);
  bool CheckContains(const std::string& actual, const std::string& substring, MemoryOutStream& stream);

  bool CheckStartsWith(const char* actual, const char* prefix, MemoryOutStream& stream);
  bool CheckStartsWith(const std::string& actual, const char* prefix, MemoryOutStream& stream);
  bool CheckStartsWith(const std::string& actual, const std::string& prefix, MemoryOutStream& stream);

  bool CheckDoesNotStartWith(const char* actual, const char* prefix, MemoryOutStream& stream);
  bool CheckDoesNotStartWith(const std::string& actual, const char* prefix, MemoryOutStream& stream);
  bool CheckDoesNotStartWith(const std::string& actual, const std::string& prefix, MemoryOutStream& stream);

  template<typename Actual, typename Expected>
  bool CheckArrayEqual(Actual const& actual, Expected const& expected, unsigned const count,
                       MemoryOutStream& stream)
  {
    bool equal = true;
    for (int i = 0; i < count; ++i)
      equal &= (actual[i] == expected[i]);

    if (equal)
      return true;

    stream << "Expected [ ";

    for (int expectedIndex = 0; expectedIndex < count; ++expectedIndex)
      stream << expected[expectedIndex] << " ";

    stream << "] but was [ ";

    for (int actualIndex = 0; actualIndex < count; ++actualIndex)
      stream << actual[actualIndex] << " ";

    stream << "]";

    return false;
  }

  template<typename Actual, typename Expected, typename Tolerance>
  bool CheckArrayClose(Actual const& actual, Expected const& expected, unsigned const count,
                       Tolerance const& tolerance, MemoryOutStream& stream)
  {
    bool equal = true;
    for (int i = 0; i < count; ++i)
      equal &= AreClose(actual[i], expected[i], tolerance);

    if (equal)
      return true;

    stream << "Expected [ ";

    for (int expectedIndex = 0; expectedIndex < count; ++expectedIndex)
      stream << expected[expectedIndex] << " ";

    stream << "] +/- " << tolerance << " but was [ ";

    for (int actualIndex = 0; actualIndex < count; ++actualIndex)
      stream << actual[actualIndex] << " ";

    stream << "]";

    return false;
  }

  template<class ActualType, class ExpectedType>
  struct ActualExpected
  {
    explicit ActualExpected(const ActualType& actual, const ExpectedType& expected) : actual(actual), expected(expected) {}

    bool CheckEqual(MemoryOutStream& stream) { return UnitTest::CheckEqual(this->actual, this->expected, stream); }
    bool CheckBigger(MemoryOutStream& stream) { return UnitTest::CheckBigger(this->actual, this->expected, stream); }
    template<class Tolerance>
    bool CheckClose(const Tolerance& tolerance, MemoryOutStream& stream) { return UnitTest::CheckClose(this->actual, this->expected, tolerance, stream); }
    template<class Tolerance>
    bool CheckClosePercentage(const Tolerance& tolerance, MemoryOutStream& stream) { return UnitTest::CheckClosePercentage(this->actual, this->expected, tolerance, stream); }
    bool CheckContains(MemoryOutStream& stream) { return UnitTest::CheckContains(this->actual, this->expected, stream); }
    bool CheckStartsWith(MemoryOutStream& stream) { return UnitTest::CheckStartsWith(this->actual, this->expected, stream); }
    bool CheckDoesNotStartWith(MemoryOutStream& stream) { return UnitTest::CheckDoesNotStartWith(this->actual, this->expected, stream); }
    bool CheckArrayEqual(unsigned const count, MemoryOutStream& stream) { return UnitTest::CheckArrayEqual(this->actual, this->expected, count, stream); }
    template<class Tolerance>
    bool CheckArrayClose(unsigned const count, const Tolerance& tolerance, MemoryOutStream& stream) { return UnitTest::CheckArrayClose(this->actual, this->expected, count, tolerance, stream); }

    const ActualType& actual;
    const ExpectedType& expected;
  };

  template<class ActualType>
  struct Actual 
  {
    explicit Actual(const ActualType& actual) : actual(actual) {}

    template<class ExpectedType>
    ActualExpected<ActualType, ExpectedType> Expected(const ExpectedType& expected)
    {
      return ActualExpected<ActualType, ExpectedType>(this->actual, expected);
    }

    const ActualType& actual;
  };

  template<class ActualType>
  Actual<ActualType> MakeActual(const ActualType& actual)
  {
    return Actual<ActualType>(actual);
  }
}
