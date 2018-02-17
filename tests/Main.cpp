#include <cxxopts/cxxopts.hpp>
#include <StringUtil.hpp>
#include <Test.h>
#include <TestList.h>
#include <TestReporter.h>
#include <TestResults.h>
#include <TestRunner.h>

class TestReporter : public UnitTest::TestReporter
{
public:
  TestReporter() = default;

  void ReportTestStart(const UnitTest::TestDetails& test) override
  {
    printf("Started test %s/%s\n", test.suiteName, test.testName);
    fflush(stdout);
  }

  void ReportFailure(const UnitTest::TestDetails& test, const char* failure) override
  {
    std::string checkId;
    if (test.checkId && *test.checkId)
      checkId = StringUtil::ssprintf(" [%s]", test.checkId);

    printf("Test %s/%s%s failed: %s:%d: %s\n",
           test.suiteName, test.testName, checkId.c_str(), test.filename, test.lineNumber, failure);
    fflush(stdout);
  }

  void ReportTestFinish(const UnitTest::TestDetails& test, float secondsElapsed) override
  {
    printf("Test %s/%s finished; elapsed %.4fs\n", test.suiteName, test.testName, secondsElapsed);
    fflush(stdout);
  }

  void ReportSummary(int totalTestCount, int failedTestCount, int failureCount, float secondsElapsed) override
  {
    if (failureCount > 0)
      printf("\nFAILURE: %d out of %d tests failed (%d failures).\n\n", failedTestCount, totalTestCount, failureCount);
    else
      printf("\nSuccess: %d tests passed.\n", totalTestCount);

    printf("Test time: %.2f seconds.\n", secondsElapsed);
    fflush(stdout);
  }
};

class TestSelector
{
public:
  void reset() const
  {
    this->reachedStart = false;
  }

  bool shouldRun(const std::string& test, const std::string& suite) const
  {
    bool result = this->shouldRunInternal(test, suite);
    if (this->invert)
      result = !result;
    return result;
  }

  bool shouldRunInternal(const std::string& test, const std::string& suite) const
  {
    if (!this->testName.empty() && this->testName != test)
      return false;
    if (!this->testNames.empty() && std::find(this->testNames.begin(), this->testNames.end(), test) == this->testNames.end())
      return false;

    if (!this->suiteName.empty() && this->suiteName != suite)
      return false;
    if (!this->suiteNames.empty() && std::find(this->suiteNames.begin(), this->suiteNames.end(), suite) == this->suiteNames.end())
      return false;

    if (!this->searchText.empty() && (test.find(this->searchText) == std::string::npos && suite.find(this->searchText) == std::string::npos))
      return false;
    if (!this->testSearchText.empty() && test.find(this->testSearchText) == std::string::npos)
      return false;
    if (!this->suiteSearchText.empty() && suite.find(this->suiteSearchText) == std::string::npos)
      return false;
    if (!this->from.empty())
    {
      if (StringUtil::ssprintf("%s/%s", suite.c_str(), test.c_str()) == this->from)
        this->reachedStart = true;
      if (!this->reachedStart)
        return false;
    }
    if (!this->after.empty())
    {
      if (!this->reachedStart)
      {
        if (StringUtil::ssprintf("%s/%s", suite.c_str(), test.c_str()) == this->after)
          this->reachedStart = true;
        return false;
      }
    }
    return true;
  }

  void logSelection() const
  {
    printf("Test selection:\n");
    if (!this->testName.empty())
      printf("Only test %s\n", testName.c_str());
    if (!this->testNames.empty())
    {
      std::string tests = this->testNames[0];
      for (size_t i = 1; i < this->testNames.size(); ++i)
        tests += ", " + this->testNames[i];
      printf("Tests %s\n", tests.c_str());
    }

    if (!this->suiteName.empty())
      printf("Only suite %s\n", suiteName.c_str());
    if (!this->suiteNames.empty())
    {
      std::string suites = this->suiteNames[0];
      for (size_t i = 1; i < this->suiteNames.size(); ++i)
        suites += ", " + this->suiteNames[i];
      printf("Suites %s\n", suites.c_str());
    }

    if (!this->searchText.empty())
      printf("Suite or test contains %s\n", this->searchText.c_str());
    if (!this->testSearchText.empty())
      printf("Test contains %s\n", this->testSearchText.c_str());
    if (!this->suiteSearchText.empty())
      printf("Suite contains %s\n", this->suiteSearchText.c_str());
    if (!this->from.empty())
      printf("From %s\n", this->from.c_str());
    if (!this->after.empty())
      printf("After %s\n", this->after.c_str());
    if (this->invert)
      printf("Invert\n");
  }

  std::string testName;
  std::vector<std::string> testNames;
  std::string suiteName;
  std::vector<std::string> suiteNames;
  std::string searchText;
  std::string testSearchText;
  std::string suiteSearchText;
  std::string from;
  std::string after;
  bool invert = false;
  mutable bool reachedStart = false;
};

class TestRunner : UnitTest::TestRunner
{
public:
  TestRunner(UnitTest::TestReporter& reporter, bool exitOnFailure)
    : UnitTest::TestRunner(reporter)
    , exitOnFailure(exitOnFailure)
  {}

  /// Run list of tests filtereed by TestSelector
  /// and return true if other tests can be run
  /// After the last runList is called, the finishe method must be used.
  bool runList(const TestSelector& selector,
               const UnitTest::TestList& list)
  {
    selector.reset();

    for (UnitTest::Test* test: list.tests)
    {
      if (!selector.shouldRun(test->m_details.testName, test->m_details.suiteName))
        continue;

      RunTest(this->m_result, test, 0);

      if (this->exitOnFailure && this->m_result->GetFailureCount())
        return false;
    }

    return true;
  }

  int finish()
  {
    return this->Finish();
  }

private:
  bool exitOnFailure;
};

namespace po = cxxopts;
int main(int argc, char** argv)
{
  po::Options variables_map("tests");
  variables_map.add_options("All")
    ("help", "display help")
    ("t,test", "run single test", po::value<std::string>())
    ("tests", "run multiple tests - a CSV list of test names", po::value<std::string>())
    ("s,suite", "run single suite", po::value<std::string>())
    ("suites", "run multiple suites - a CSV list of suite names", po::value<std::string>())
    ("from", "suite/test run all tests starting with this suite/test", po::value<std::string>())
    ("after", "suite/test run all tests starting with the next suite/test after specified one", po::value<std::string>())
    ("contains", "run all tests that contain the given string in the test or suite name", po::value<std::string>())
    ("test-contains", "run all tests that contain the given string in the test name", po::value<std::string>())
    ("suite-contains", "run all suites that contain the given string in the suite name", po::value<std::string>())
    ("V,invert-selection", "Invert selection of test done by the previous parameters")
    ("exit-on-failure", "Exit after a first failed test.");

  try
  {
    variables_map.parse(argc, argv);
  }
  catch (const cxxopts::OptionParseException & e)
  {
    printf("ERROR: %s\n\n", e.what());
    return 1;
  }

  if (variables_map.count("help"))
  {
    printf("%s\n", variables_map.help({"", "All"}).c_str());
    fflush(stdout);
    return 0;
  }

  TestSelector testSelector;
  if (variables_map.count("test"))
    testSelector.testName = variables_map["test"].as<std::string>();
  if (variables_map.count("tests"))
    testSelector.testNames = StringUtil::split(",", variables_map["tests"].as<std::string>());
  if (variables_map.count("suite"))
    testSelector.suiteName = variables_map["suite"].as<std::string>();
  if (variables_map.count("suites"))
    testSelector.suiteNames = StringUtil::split(",", variables_map["suites"].as<std::string>());
  if (variables_map.count("from"))
    testSelector.from = variables_map["from"].as<std::string>();
  if (variables_map.count("after"))
    testSelector.after = variables_map["after"].as<std::string>();
  if (variables_map.count("contains"))
    testSelector.searchText = variables_map["contains"].as<std::string>();
  if (variables_map.count("test-contains"))
    testSelector.testSearchText = variables_map["test-contains"].as<std::string>();
  if (variables_map.count("suite-contains"))
    testSelector.suiteSearchText = variables_map["suite-contains"].as<std::string>();
  if (variables_map.count("invert-selection"))
    testSelector.invert = variables_map["invert-selection"].as<bool>();


  try
  {
    UnitTest::Test::GetTestList().sort();
    TestReporter reporter;
    TestRunner runner(reporter, variables_map["exit-on-failure"].as<bool>());
    if (!runner.runList(testSelector, UnitTest::Test::GetTestList()))
      return runner.finish();

    return runner.finish();
  }
  catch (const std::exception& e)
  {
    printf("Unhandled exeption: %s.", e.what());
    return 1;
  }

  return 0;
}
