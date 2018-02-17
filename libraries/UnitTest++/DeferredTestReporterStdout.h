#ifndef UNITTEST_DEFERREDTESTREPORTERSTDOUT_H
#define UNITTEST_DEFERREDTESTREPORTERSTDOUT_H

#include "TestReporter.h"
#include "DeferredTestResult.h"
#include "Config.h"

#include <vector>

namespace UnitTest {

class DeferredTestReporterStdout : public TestReporter
{
protected:
    virtual void ReportTestStart(TestDetails const& test) override;
    virtual void ReportFailure(TestDetails const& test, char const* failure) override;
    virtual void ReportTestFinish(TestDetails const& test, float secondsElapsed) override;
    virtual void ReportSummary(int totalTestCount, int failedTestCount, int failureCount, float secondsElapsed) override;

    typedef std::vector< DeferredTestResult > DeferredTestResultList;
    DeferredTestResultList& GetResults();

private:
    DeferredTestResultList m_results;
};

}

#endif 
