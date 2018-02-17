#pragma once
#include <cstdint>
#include <vector>
struct lua_State;

class LuaPrinter
{
public:
  LuaPrinter();
  LuaPrinter(const LuaPrinter&) = delete;
  LuaPrinter(LuaPrinter&&) = delete;
  ~LuaPrinter();

  int32_t luaIndex(lua_State* L);
  int32_t luaNewIndex(lua_State* L);

  int32_t luaPrint(lua_State* L);

  // Execute a command and print the result to the printer
  std::string runCommand(const std::string& command);
  // Execute a command and return the last item added to the console if any
  std::string scriptCommand(const std::string& command);

  const std::vector<std::string>& getLines() const { return this->lines; }
  const std::string& getLastLine() const;

  LuaPrinter& operator=(const LuaPrinter&) = delete;
  LuaPrinter& operator=(LuaPrinter&&) = delete;

private:
  void doString(const std::string& string);

  lua_State* L = nullptr;
  std::vector<std::string> lines;

  static bool staticInit();
  static bool loaded;
};
