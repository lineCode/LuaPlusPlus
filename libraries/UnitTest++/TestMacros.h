#ifndef UNITTEST_TESTMACROS_H
#define UNITTEST_TESTMACROS_H

#include "Config.h"
#include <Fixtures.h>

#ifdef TEST
    #error UnitTest++ redefines TEST
#endif

#ifdef TEST_FIXTURE_EX
    #error UnitTest++ redefines TEST_FIXTURE_EX
#endif

#define SUITE(Name)                                                             \
    namespace Suite##Name {                                                     \
        namespace UnitTestSuite {                                               \
            inline char const* GetSuiteName () {                                \
                return #Name ;                                                  \
            }                                                                   \
        }                                                                       \
    }                                                                           \
    namespace Suite##Name

#define TEST_FIXTURE_EX(Fixture, Name, List)                                    \
    class Fixture##Name##Helper : public Fixture, public UnitTest::FixtureHelperBase \
    {                                                                           \
    public:                                                                     \
        Fixture##Name##Helper() {}                                              \
        void Run() override;                                                    \
    private:                                                                    \
        Fixture##Name##Helper(Fixture##Name##Helper const&) = delete;           \
        Fixture##Name##Helper& operator=(Fixture##Name##Helper const&) = delete; \
    };                                                                          \
                                                                                \
    class Test##Fixture##Name : public UnitTest::Test                           \
    {                                                                           \
    public:                                                                     \
        Test##Fixture##Name() : Test(#Name, UnitTestSuite::GetSuiteName(), __FILE__, __LINE__) {} \
    private:                                                                    \
        UnitTest::FixtureHelperBase* CreateFixtureHelper() const override;      \
    } test##Fixture##Name##Instance;                                            \
                                                                                \
    UnitTest::ListAdder adder##Fixture##Name (List, &test##Fixture##Name##Instance); \
                                                                                \
    UnitTest::FixtureHelperBase* Test##Fixture##Name::CreateFixtureHelper() const \
    {                                                                           \
        return new Fixture##Name##Helper();                                     \
    }                                                                           \
                                                                                \
    void Fixture##Name##Helper::Run()

#define TEST(Name) TEST_FIXTURE_EX(UnitTest__NullFixture, Name, UnitTest::Test::GetTestList())

#define TEST_FIXTURE(Fixture,Name) TEST_FIXTURE_EX(Fixture, Name, UnitTest::Test::GetTestList())


#endif
