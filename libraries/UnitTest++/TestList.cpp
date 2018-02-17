#include "TestList.h"
#include "Test.h"
#include <algorithm>

namespace UnitTest
{
  void TestList::Add(Test* test)
  {
    this->tests.push_back(test);
  }

  void TestList::sort()
  {
    std::sort(this->tests.begin(), this->tests.end(), [](const Test* a, const Test* b) { return *a < *b; });
  }

  ListAdder::ListAdder(TestList& list, Test* test)
  {
    list.Add(test);
  }
}
