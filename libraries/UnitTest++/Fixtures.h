#ifndef UNITTEST_FIXTURES_H
#define UNITTEST_FIXTURES_H

namespace UnitTest {

class FixtureHelperBase
{
public:
    virtual ~FixtureHelperBase() noexcept(false) {}
    virtual void Run() = 0;
};

}

// This class must be out of a namespace because of some macro shenanigans
class UnitTest__NullFixture {};

#endif
