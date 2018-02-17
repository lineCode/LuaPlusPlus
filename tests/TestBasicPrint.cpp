#include <LuaPrinter.hpp>
#include <UnitTest++.h>

SUITE(LuaPrint)
{
  TEST(BasicPrint)
  {
    LuaPrinter printer;

    CHECK_EQUAL(printer.runCommand("'basic print'"), "basic print");
    CHECK_EQUAL(printer.runCommand("'Something else'"), "Something else");
    CHECK_EQUAL(printer.runCommand("'And it is finished..'"), "And it is finished..");
  }
}
