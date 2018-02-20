#include <ltype.hpp>
#include <lualib.hpp>
#include <lauxlib.hpp>
#include <llimits.hpp>

const char* LuaType::toString(Basic value)
{
  lua_assert(LuaType(value).asBasic() == value);

  switch (value)
  {
    case Basic::Nil: return "nil";
    case Basic::Boolean: return "boolean";
    case Basic::LightUserData: return "userdata";
    case Basic::Number: return "number";
    case Basic::String: return "string";
    case Basic::Table: return "table";
    case Basic::Function: return "function";
    case Basic::UserData: return "userdata";
    case Basic::Thread: return "thread";
  }

  abort(); // Total failure == bugged code somewhere
}

const char* LuaType::toString(Variant value)
{
  lua_assert(LuaType(value).asVariant() == value);

  // Switch serves 2 purposes here:
  // 1. To convert the type to a string
  // 2. To ensure every type value is unique
  switch (value)
  {
    case Variant::None: return "no value";
    case Variant::Nil:
    case Variant::Boolean:
    case Variant::LightUserData:
    case Variant::Number:
    case Variant::String:
    case Variant::Table:
    case Variant::Function:
    case Variant::UserData:
    case Variant::Thread:
      return LuaType::toString(LuaType(value).asBasic());
    case Variant::FunctionPrototype: return "proto";
    case Variant::DeadKey: return "dead key";
    //case Variant::LuaFunctionClosure: return "lua function closure";
    case Variant::LightCFunction: return "light C function";
    case Variant::CFunctionClosure: return "C function closure";
    //case Variant::ShortString: return "short string";
    case Variant::LongString: return "long string";
    //case Variant::FloatNumber: return "float number";
    case Variant::IntNumber: return "int number";
  }

  // These types are aliases and aren't included in the above switch
  static_assert(Variant::Function == Variant::LuaFunctionClosure);
  static_assert(Variant::String == Variant::ShortString);
  static_assert(Variant::Number == Variant::FloatNumber);

  abort(); // Total failure == bugged code somewhere
}

LuaType LuaType::fromRaw(lua_State* L, DataType value)
{
  switch (Variant(value))
  {
    case Variant::None:
    case Variant::Nil:
    case Variant::Boolean:
    case Variant::LightUserData:
    case Variant::Table:
    case Variant::UserData:
    case Variant::Thread:
    case Variant::FunctionPrototype:
    case Variant::DeadKey:
    case Variant::LuaFunctionClosure:
    case Variant::LightCFunction:
    case Variant::CFunctionClosure:
    case Variant::ShortString:
    case Variant::LongString:
    case Variant::FloatNumber:
    case Variant::IntNumber:
      return LuaType(Variant(value));
  }

  if (L)
    luaL_error(L, "Invalid raw LuaType value: %i.", value);
  abort();
}
