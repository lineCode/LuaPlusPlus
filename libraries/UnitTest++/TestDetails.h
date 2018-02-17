#ifndef UNITTEST_TESTDETAILS_H
#define UNITTEST_TESTDETAILS_H

namespace UnitTest {

class TestDetails
{
public:
    TestDetails(char const* testName, char const* suiteName, char const* filename, int lineNumber, char const* checkId = 0);
    TestDetails(const TestDetails& details, char const* filename, int lineNumber, char const* checkId);

    char const* const suiteName;
    char const* const testName;
    char const* const filename;
    int const lineNumber;
    char const* const checkId;

    TestDetails(TestDetails const&); // Why is it public? --> http://gcc.gnu.org/bugs.html#cxx_rvalbind
private:
    TestDetails& operator=(TestDetails const&);
};

}

#endif
