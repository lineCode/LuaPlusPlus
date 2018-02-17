#ifndef UNITTEST_TEST_H
#define UNITTEST_TEST_H

#include "Fixtures.h"
#include "TestDetails.h"

#ifdef UNITTEST_POSIX
  #include "Posix/SignalTranslator.h"
#else
  #define UNITTEST_THROW_SIGNALS (void)0;
#endif

namespace UnitTest
{
  class TestResults;
  class TestList;

  class Test
  {
  public:
    explicit Test(char const* testName, char const* suiteName = "DefaultSuite", char const* filename = "", int lineNumber = 0);

    virtual ~Test()
    {};

    virtual void Run();

    TestDetails const m_details;
    mutable bool m_timeConstraintExempt;

    static TestList& GetTestList();

    bool operator<(const Test& test) const;

  protected:
    virtual FixtureHelperBase* CreateFixtureHelper() const
    {
      return nullptr;
    };

    Test(Test const&) = delete;
    Test& operator =(Test const&) = delete;

    void HandleExceptions(const char* context);
  };
}

#endif
