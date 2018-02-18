#pragma once
#include <stdexcept>
#include <lua.hpp>

class lua_exception : public std::runtime_error
{
public:
  lua_exception(int status, const char* error)
    : std::runtime_error(error ? error : "unknown")
    , status(status)
  {}

  int status = LUA_OK;
};

class lua_unhandled_exception : public lua_exception
{
public:
  lua_unhandled_exception(int status, const char* error)
    : lua_exception(status, error)
  {}
};
