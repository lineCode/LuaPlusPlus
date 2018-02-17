#include "TestDetails.h"

namespace UnitTest {

TestDetails::TestDetails(char const* testName_, char const* suiteName_, char const* filename_, int lineNumber_, char const* checkId_)
    : suiteName(suiteName_)
    , testName(testName_)
    , filename(filename_)
    , lineNumber(lineNumber_)
    , checkId((checkId_ && *checkId_) ? checkId_ : 0)
{
}

TestDetails::TestDetails(const TestDetails& details, char const* filename_, int lineNumber_, char const* checkId_)
    : suiteName(details.suiteName)
    , testName(details.testName)
    , filename(filename_)
    , lineNumber(lineNumber_)
    , checkId((checkId_ && *checkId_) ? checkId_ : 0)
{
}


}
