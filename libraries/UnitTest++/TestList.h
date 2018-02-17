#ifndef UNITTEST_TESTLIST_H
#define UNITTEST_TESTLIST_H
#include <vector>

namespace UnitTest
{
  class Test;

  class TestList
  {
  public:
    using TestMapping = std::vector<Test*>;
    TestList() = default;
    void Add(Test* test);
    void sort();

    TestMapping tests;
  };

  class ListAdder
  {
  public:
    ListAdder(TestList& list, Test* test);
  };
}


#endif
