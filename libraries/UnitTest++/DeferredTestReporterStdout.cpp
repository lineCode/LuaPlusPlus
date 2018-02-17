#include "DeferredTestReporterStdout.h"
#include <cstdio>

#include "TestDetails.h"

// cstdio doesn't pull in namespace std on VC6, so we do it here.
#if defined(_MSC_VER) && (_MSC_VER == 1200)
	namespace std {}
#endif

namespace UnitTest {

void DeferredTestReporterStdout::ReportFailure(TestDetails const& details, char const* failure)
{
    DeferredTestResult& r = m_results.back();
    r.failed = true;
    r.failures.push_back(DeferredTestResult::Failure(details.lineNumber, failure));
    r.failureFile = details.filename;

#if defined(__APPLE__) || defined(__GNUG__)
    std::printf("%s:%d: ", details.filename, details.lineNumber);
#else
    std::printf("%s(%d): ", details.filename, details.lineNumber);
#endif

    if (details.checkId && *details.checkId)
        std::printf("[%s] ", details.checkId);

    std::printf("error: Failure in %s: %s\n", details.testName, failure);
}

void DeferredTestReporterStdout::ReportTestStart(TestDetails const& details)
{
  m_results.push_back(DeferredTestResult(details.suiteName, details.testName));
  printf("[%s/%s] ", details.suiteName, details.testName);
  fflush(stdout);
}

void DeferredTestReporterStdout::ReportTestFinish(TestDetails const& test, float secondsElapsed)
{
  (void)test;
  DeferredTestResult& r = m_results.back();
  r.timeElapsed = secondsElapsed;
  printf("%.4fs\n", secondsElapsed);
}

void DeferredTestReporterStdout::ReportSummary(int const totalTestCount, int const failedTestCount,
                                       int const failureCount, float secondsElapsed)
{
	using namespace std;

    if (failureCount > 0)
    {
        printf("FAILURE: %d out of %d tests failed (%d failures).\n\n", failedTestCount, totalTestCount, failureCount);
        printf("Failed tests:\n");
        for (const auto& result: GetResults())
        {
          if (result.failed)
          {
            printf("[%s/%s]\n", result.suiteName.c_str(), result.testName.c_str());
            for (const auto& failure: result.failures)
            {
              printf(" %s:%d: %s\n", result.failureFile.c_str(), failure.first, failure.second.c_str());
            }
          }
        }
    }
    else
        printf("Success: %d tests passed.\n", totalTestCount);

    printf("Test time: %.2f seconds.\n", secondsElapsed);
}

DeferredTestReporterStdout::DeferredTestResultList& DeferredTestReporterStdout::GetResults()
{
    return m_results;
}

}
