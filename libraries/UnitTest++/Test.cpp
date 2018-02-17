#include "Config.h"
#include "Test.h"
#include "TestList.h"
#include "TestResults.h"
#include "AssertException.h"
#include "TerminateTestException.h"
#include "MemoryOutStream.h"
#include "CurrentTest.h"
#include <cstring>

namespace UnitTest {

TestList& Test::GetTestList()
{
    static TestList s_list;
    return s_list;
}

Test::Test(char const* testName, char const* suiteName, char const* filename, int lineNumber)
    : m_details(testName, suiteName, filename, lineNumber)
    , m_timeConstraintExempt(false)
{
}

void Test::Run()
{
    CurrentTest::Details() = &m_details;

    FixtureHelperBase* fixtureHelper = nullptr;

    try
    {
        UNITTEST_THROW_SIGNALS
        fixtureHelper = CreateFixtureHelper();
    }
    catch (...)
    {
        HandleExceptions("creating fixture");
        return;
    }

    try
    {
        UNITTEST_THROW_SIGNALS
        fixtureHelper->Run();
    }
    catch (...)
    {
        HandleExceptions("running test");
    }

    try
    {
        UNITTEST_THROW_SIGNALS
        delete fixtureHelper;
    }
    catch (...)
    {
        HandleExceptions("tearing down fixture");
    }
}

bool Test::operator<(const Test& test) const
{
  if (int suiteNameDiff = strcmp(this->m_details.suiteName, test.m_details.suiteName))
    return suiteNameDiff < 0;
  if (int testNameDiff = strcmp(this->m_details.testName, test.m_details.testName))
    return testNameDiff < 0;
  if (int fileNameDiff = strcmp(this->m_details.filename, test.m_details.filename))
    return fileNameDiff < 0;
  return this->m_details.lineNumber < test.m_details.lineNumber;
}

void Test::HandleExceptions(const char* context)
{
    try
    {
        try
        {
            // Rethrow the previously catched exception to catch it again.
            throw;
        }
        catch (AssertException const& e)
        {
            CurrentTest::Results()->OnTestFailure(
                TestDetails(m_details.testName, m_details.suiteName, e.Filename(), e.LineNumber()), e.what());
        }
        catch (TerminateTestException const&)
        {
            return;
        }
        catch (std::exception const& e)
        {
            MemoryOutStream stream;
            stream << "Unhandled exception while " << context << ": " << e.what();
            CurrentTest::Results()->OnTestFailure(m_details, stream.GetText());
        }
        catch (...)
        {
            MemoryOutStream stream;
            stream << "Unhandled exception while " << context;
            CurrentTest::Results()->OnTestFailure(m_details, stream.GetText());
        }
    }
    catch (TerminateTestException const&)
    {
        return;
    }
}

}
