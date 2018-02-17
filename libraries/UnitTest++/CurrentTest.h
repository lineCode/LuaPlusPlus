#ifndef UNITTEST_CURRENTTESTRESULTS_H
#define UNITTEST_CURRENTTESTRESULTS_H

namespace UnitTest {

class TestResults;
class TestDetails;

namespace CurrentTest
{
	TestResults*& Results();
	const TestDetails*& Details();
  TestDetails Details(char const* filename, int lineNumber, char const* checkId);

  void OnTestFailure(char const* filename, int lineNumber, char const* checkId, char const* failure);
}

}

#endif
