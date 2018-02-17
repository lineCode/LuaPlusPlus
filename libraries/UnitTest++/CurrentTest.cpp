#include "CurrentTest.h"
#include "TestResults.h"
#include "TestDetails.h"
#include <cstddef>

namespace UnitTest {

TestResults*& CurrentTest::Results()
{
	static TestResults* testResults = NULL;
	return testResults;
}

const TestDetails*& CurrentTest::Details()
{
	static const TestDetails* testDetails = NULL;
	return testDetails;
}

void CurrentTest::OnTestFailure(char const* filename, int lineNumber, char const* checkId, char const* failure)
{
  Results()->OnTestFailure(TestDetails(*Details(), filename, lineNumber, checkId), failure);
}

}
