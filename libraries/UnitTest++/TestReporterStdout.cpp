#include "TestReporterStdout.h"
#include <cstdio>

#include "TestDetails.h"

// cstdio doesn't pull in namespace std on VC6, so we do it here.
#if defined(_MSC_VER) && (_MSC_VER == 1200)
	namespace std {}
#endif

namespace UnitTest {

void TestReporterStdout::ReportFailure(TestDetails const& details, char const* failure)
{
#if defined(__APPLE__) || defined(__GNUG__)
    std::printf("%s:%d: ", details.filename, details.lineNumber);
#else
    std::printf("%s(%d): ", details.filename, details.lineNumber);
#endif

    if (details.checkId && *details.checkId)
        std::printf("[%s] ", details.checkId);

    std::printf("error: Failure in %s: %s\n", details.testName, failure);
}

void TestReporterStdout::ReportTestStart(TestDetails const& test)
{
  printf("[%s/%s] ", test.suiteName, test.testName);
  fflush(stdout);
}

void TestReporterStdout::ReportTestFinish(TestDetails const& test, float secondsElapsed)
{
  (void)test;
  printf("%.4fs\n", secondsElapsed);
}

void TestReporterStdout::ReportSummary(int const totalTestCount, int const failedTestCount,
                                       int const failureCount, float secondsElapsed)
{
	using namespace std;

    if (failureCount > 0)
        printf("FAILURE: %d out of %d tests failed (%d failures).\n", failedTestCount, totalTestCount, failureCount);
    else
        printf("Success: %d tests passed.\n", totalTestCount);

    printf("Test time: %.2f seconds.\n", secondsElapsed);
}

}
