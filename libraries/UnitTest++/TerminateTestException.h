#ifndef UNITTEST_TERMINATETESTEXCEPTION_H
#define UNITTEST_TERMINATETESTEXCEPTION_H

#include <stdexcept>

namespace UnitTest {

class TerminateTestException : public std::runtime_error
{
public:
  TerminateTestException() : std::runtime_error("") {}
};

}

#endif
