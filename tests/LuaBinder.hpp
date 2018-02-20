#pragma once
#include <cassert>
#include <lauxlib.hpp>
#include <lua.hpp>
#include <map>
#include <stdexcept>

/** Helper class to register instance methods as callbacks to lua. */
template<typename T>
class LuaBinder
{
public:
  using PtFunc = int32_t (T::*)(lua_State*);
  class FuncWrapper
  {
  public:
    FuncWrapper() = default;
    FuncWrapper(PtFunc ptFunc) : ptFunc(ptFunc) {}

    PtFunc ptFunc = nullptr;
  };

  using FuncWrapperMap = std::map<std::string, FuncWrapper>;

  /** Make a table into a object that points to obj. The new object
   * gets a metatable and a __self lighuserdata pointer to obj. */
  static void registerObject(lua_State* L, T* obj, int32_t tableIndex)
  {
#ifdef DEBUG
    int32_t inputTop = lua_gettop(L);
#endif
    assert(lua_istable(L, tableIndex));

    if (tableIndex < 0)
      tableIndex += lua_gettop(L) + 1; // make the table index absolute
    assert(lua_istable(L, tableIndex));

    // set the __self pointer
    lua_pushlightuserdata(L, obj);
    lua_setfield(L, tableIndex, "__self");

    // create metatable
    lua_createtable(L, 0, static_cast<int>(LuaBinder<T>::getMetaFunctionMap().size())); // meta-methods

    // fill meta functions
    for (auto& pair: LuaBinder<T>::getMetaFunctionMap())
      LuaBinder<T>::registerWrapper(L, obj, &pair.second, pair.first, -1);

    // connect metatable
    lua_setmetatable(L, tableIndex);

    assert(lua_gettop(L) == inputTop);
  }

  static void registerHandler(FuncWrapperMap& map, std::string&& name, PtFunc method)
  {
    if (map.find(name) != map.end())
      throw std::runtime_error("Handler (" + name + ") registered twice.");
    map[std::move(name)] = FuncWrapper(method);
  }

  static void registerMethod(std::string&& name, PtFunc method)
  {
    LuaBinder<T>::registerHandler(LuaBinder<T>::getFunctionMap(), std::move(name), method);
  }

  static void registerMetaMethod(std::string&& name, PtFunc method)
  {
    LuaBinder<T>::registerHandler(LuaBinder<T>::getMetaFunctionMap(), std::move(name), method);
  }

  static void registerRead(std::string&& name, PtFunc method)
  {
    LuaBinder<T>::registerHandler(LuaBinder<T>::getReadIndexMap(), std::move(name), method);
  }

  static void registerWrite(std::string&& name, PtFunc method)
  {
    LuaBinder<T>::registerHandler(LuaBinder<T>::getWriteIndexMap(), std::move(name), method);
  }

  static T* getObject(lua_State* L, int32_t index)
  {
    if (!lua_istable(L, index))
      return nullptr;
#ifdef DEBUG
    int32_t inputTop = lua_gettop(L);
#endif
    lua_getfield(L, index, "__self");
    if (!lua_islightuserdata(L, -1))
      return nullptr;
    T* ret = static_cast<T*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    assert(lua_gettop(L) == inputTop);
    return ret;
  }

  /** Register method directly to an instance. */
  static void registerWrapper(lua_State* L,
                              T* obj,
                              FuncWrapper* wrapper,
                              const std::string& name,
                              int32_t index,
                              bool adjustIndex = true)
  {
    LuaBinder<T>::pushCallWrapperClosure(L, obj, wrapper);
    int32_t adjustedIndex = adjustIndex ? (index < 0 ? index - 1 : index) : index;
    lua_setfield(L, adjustedIndex, name.c_str());
  }

  /** Locate the method in the functions table and return c closure that uses
   * the callWrapper mechanism. */
  static bool findAndPushMethod(lua_State* L, T* obj, const std::string& key, FuncWrapperMap& funcWrapperMap)
  {
    auto it = funcWrapperMap.find(key);
    if (it == funcWrapperMap.end())
      return false;

    LuaBinder<T>::pushCallWrapperClosure(L, obj, &it->second);
    return true;
  }

  /** Locate the method in the functions table, call it and return its return value.
   * returns -1 if the function is not found. */
  static int32_t findAndCallMethod(lua_State* L, T* obj, const std::string& key, FuncWrapperMap& funcWrapperMap)
  {
    auto it = funcWrapperMap.find(key);
    if (it == funcWrapperMap.end())
      return -1;

    // When a method is called directly we need to remove the LuaObject and method name from the stack ourselves
    // When a closure is pushed to the Lua state Lua handles the extra items on the stack.
    lua_remove(L, 1); // Remove the method name
    lua_remove(L, 1); // Remove the LuaObject wrapper table
    return (obj->*(it->second.ptFunc))(L);
  }

  static void registerCommonMetaMethods()
  {
    LuaBinder<T>::registerMetaMethod("__index", &T::luaIndex);
    LuaBinder<T>::registerMetaMethod("__newindex", &T::luaNewIndex);
    LuaBinder<T>::registerMetaMethod("__gc", &T::collect);
  }

  [[noreturn]]
  static void writableKeyNotFound(const std::string& key)
  {
    if (LuaBinder<T>::getReadIndexMap().find(key) != LuaBinder<T>::getReadIndexMap().end())
      throw std::runtime_error("Key is read only: " + key);
    LuaBinder<T>::keyNotFound(key);
  }

  [[noreturn]]
  static void keyNotFound(const std::string& key)
  {
    throw std::runtime_error("Key not found: " + key);
  }

  static const std::string& parseKey(lua_State* L, int32_t argCount)
  {
    static thread_local std::string key;
    luaL_argcheck(L, lua_gettop(L) == argCount, 0, "Wrong number of arguments.");
    luaL_checktype(L, 1, LuaType::Basic::Table);

    key.clear();
    if (const char* result = luaL_checkstring(L, 2))
      key = result;
    return key;
  }

  static int32_t processCommonLuaIndex(lua_State* L, T* object, bool throwOnKeyNotFound = true)
  {
    auto& key = LuaBinder<T>::parseKey(L, 2);

    if (LuaBinder<T>::findAndPushMethod(L, object, key, LuaBinder<T>::getFunctionMap()))
      return 1;

    int32_t result = LuaBinder<T>::findAndCallMethod(L, object, key, LuaBinder<T>::getReadIndexMap());
    if (result != -1)
      return result;

    if (throwOnKeyNotFound)
      LuaBinder<T>::keyNotFound(key);
    return -1;
  }

  static int32_t processCommonLuaNewIndex(lua_State* L, T* object, bool throwOnKeyNotFound = true)
  {
    auto& key = LuaBinder<T>::parseKey(L, 3);

    int32_t result = LuaBinder<T>::findAndCallMethod(L, object, key, LuaBinder<T>::getWriteIndexMap());
    if (result != -1)
      return result;

    if (throwOnKeyNotFound)
      LuaBinder<T>::writableKeyNotFound(key);
    return -1;
  }

  static FuncWrapperMap& getFunctionMap()
  {
    static FuncWrapperMap map;
    return map;
  }

  static FuncWrapperMap& getMetaFunctionMap()
  {
    static FuncWrapperMap map;
    return map;
  }

  static FuncWrapperMap& getReadIndexMap()
  {
    static FuncWrapperMap map;
    return map;
  }

  static FuncWrapperMap& getWriteIndexMap()
  {
    static FuncWrapperMap map;
    return map;
  }

private:
  // Takes lightuserdata with pointers to object and its method,
  // calls the method with current lua state as parameter and handles any exceptions thrown.
  static int32_t callWrapper(lua_State* L)
  {
    T* obj = static_cast<T*>(lua_touserdata(L, lua_upvalueindex(1)));
    FuncWrapper* wrapper = static_cast<FuncWrapper*>(lua_touserdata(L, lua_upvalueindex(2)));
    try
    {
      if (!lua_checkstack(L, 3))
        throw std::runtime_error("Out of stack space.");

      return (obj->*(wrapper->ptFunc))(L);
    }
    catch (const std::runtime_error& e)
    {
      luaL_traceback(L, L, e.what(), 1);
      lua_error(L);
    }
  }

  static void pushCallWrapperClosure(lua_State* L, T* obj, FuncWrapper* wrapper)
  {
    lua_pushlightuserdata(L, obj);
    // We must use the wrapper here, because pointer to member cannot be converted to void*
    lua_pushlightuserdata(L, static_cast<void*>(wrapper));
    lua_pushcclosure(L, &callWrapper, 2);
  }
};
