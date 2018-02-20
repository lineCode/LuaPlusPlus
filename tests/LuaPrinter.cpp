#include <lua.hpp>
#include <LuaBinder.hpp>
#include <lualib.hpp>
#include <LuaPrinter.hpp>
#include <StringUtil.hpp>
#include <lstate.hpp>

bool LuaPrinter::loaded = LuaPrinter::staticInit();

bool LuaPrinter::staticInit()
{
  using MyBinder = LuaBinder<LuaPrinter>;

  MyBinder::registerMetaMethod("__index", &LuaPrinter::luaIndex);
  MyBinder::registerMetaMethod("__newindex", &LuaPrinter::luaNewIndex);

  MyBinder::registerMethod("print", &LuaPrinter::luaPrint);

  //MyBinder::registerRead("wrte", &LuaPrinter::luaRead);
  //MyBinder::registerWrite("read", &LuaPrinter::luaWrite);

  return true;
}

LuaPrinter::LuaPrinter()
  : state(new lua_State())
  , L(this->state.get())
{
  if (!this->L)
    throw std::runtime_error("Failed to create Lua state.");

  // Load standard libraries
  luaL_openlibs(L);

  // Register self into the Lua state
  lua_newtable(L);
  LuaBinder<LuaPrinter>::registerObject(L, this, -1);
  lua_setglobal(L, "printer");
}

LuaPrinter::~LuaPrinter() = default;

int32_t LuaPrinter::luaIndex(lua_State* L)
{
  return LuaBinder<LuaPrinter>::processCommonLuaIndex(L, this);
}

int32_t LuaPrinter::luaNewIndex(lua_State* L)
{
  return LuaBinder<LuaPrinter>::processCommonLuaNewIndex(L, this);
}

int32_t LuaPrinter::luaPrint(lua_State* L)
{
  this->lines.emplace_back(luaL_checkstring(L, -1));
  return 0;
}

const std::string& LuaPrinter::getLastLine() const
{
  if (this->lines.empty())
    throw std::runtime_error("Lines are empty.");
  return this->lines.back();
}

std::string LuaPrinter::runCommand(const std::string& command)
{
  const size_t before = this->lines.size();

  this->doString("printer.print(tostring(" + command + "))");

  if (this->lines.size() > before)
    return this->lines.back();
  return std::string();
}

std::string LuaPrinter::scriptCommand(const std::string& command)
{
  const size_t before = this->lines.size();

  this->doString(command);

  if (this->lines.size() > before)
    return this->lines.back();
  return std::string();
}

void LuaPrinter::doString(const std::string& string)
{
  if (luaL_dostring(this->L, string.c_str()))
  {
    std::string error;
    switch (lua_type(this->L, -1).asBasic())
    {
      case LuaType::Basic::Nil:
        error = "nil";
        break;
      case LuaType::Basic::Number:
        error = StringUtil::ssprintf(LUA_NUMBER_FMT, luaL_checknumber(this->L, -1));
        break;
      case LuaType::Basic::Boolean:
        error = lua_toboolean(this->L, -1) == 0 ? "false" : "true";
        break;
      case LuaType::Basic::String:
        error = luaL_checkstring(this->L, -1);
        break;
      default:
        error = "Unknown type";
    }

    throw std::runtime_error("Error: " + error);
  }
}