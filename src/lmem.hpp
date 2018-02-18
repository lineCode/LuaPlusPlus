#pragma once
/*
** $Id: lmem.h,v 1.43 2014/12/19 17:26:14 roberto Exp $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#include <cstddef>

#include <llimits.hpp>
#include <lua.hpp>
#include <lallocator.hpp>

extern void luaM_toobig(lua_State* L);
extern void luaM_addGCDebt(lua_State* L, size_t size, size_t realosize);
extern void luaM_allocFail(lua_State* L);
extern void luaM_tooMany(lua_State* L, const char* what, int32_t limit);

template<class T>
class LMem
{
public:
  /*
  ** This reallocs a vector 'b' from 'on' to 'n' elements, where
  ** each element has size 'e'. In case of arithmetic overflow of the
  ** product 'n'*'e', it raises an error (calling 'luaM_toobig'). Because
  ** 'e' is always constant, it avoids the runtime division MAX_SIZET/(e).
  **
  ** (The macro is somewhat complex to avoid warnings:  The 'sizeof'
  ** comparison avoids a runtime comparison when overflow cannot occur.
  ** The compiler should be able to optimize the real test by itself, but
  ** when it does it, it may give a warning about "comparison is always
  ** false due to limited range of data type"; the +1 tricks the compiler,
  ** avoiding this warning but also this optimization.)
  */
  static T* luaM_reallocv(lua_State* L, T* block, size_t on, size_t n, size_t e)
  {
    ((sizeof(n) >= sizeof(size_t) && cast(size_t, (n)) + 1 > MAX_SIZET / (e)) ? luaM_toobig(L) : cast_void(0));
    return luaM_realloc_(L, block, on * e, n * e);
  }

  /*
  ** Arrays of chars do not need any test
  */
  static T* luaM_reallocvchar(lua_State* L, T* block, size_t on, size_t n)
  {
    return luaM_realloc_(L, block, on * sizeof(char), n * sizeof(char));
  }

  static void luaM_freemem(lua_State* L, T* block, size_t s)
  {
    luaM_realloc_(L, block, s, 0);
  }
  static void luaM_free(lua_State* L, T* block)
  {
    luaM_realloc_(L, block, sizeof(T), 0);
  }
  static void luaM_freearray(lua_State* L, T* block, size_t n)
  {
    luaM_realloc_(L, block, n * sizeof(T), 0);
  }

  static T* luaM_malloc(lua_State* L, size_t s)
  {
    return luaM_realloc_(L, nullptr, 0, s);
  }

  static T* luaM_new(lua_State* L)
  {
    return luaM_malloc(L, sizeof(T));
  }

  static T* luaM_newvector(lua_State* L, size_t n)
  {
    return luaM_reallocv(L, nullptr, 0, n, sizeof(T));
  }

  static T* luaM_newobject(lua_State* L, size_t tag, size_t s)
  {
    return luaM_realloc_(L, nullptr, tag, s);
  }

  static void luaM_growvector(lua_State* L, T*& block, size_t nelems, int32_t& size, int32_t limit, const char* e)
  {
    if ((nelems)+1 > (size))
      block = luaM_growaux_(L, block, size, limit, e);
  }

  static void luaM_reallocvector(lua_State* L, T*& block, size_t oldn, size_t n)
  {
    block = luaM_reallocv(L, block, oldn, n, sizeof(T));
  }

  static T* luaM_growaux_(lua_State* L, T* block, int32_t& size, int32_t limit, const char* what)
  {
    constexpr size_t MINARRAYSIZE = 4;

    int32_t newsize;
    if (size >= limit / 2)
    { // cannot double it?
      if (size >= limit)  // cannot grow even a little?
        luaM_tooMany(L, what, limit);
      newsize = limit;  // still have at least one free place
    }
    else {
      newsize = (size)*2;
      if (newsize < MINARRAYSIZE)
        newsize = MINARRAYSIZE;  /* minimum size */
    }
    T* newblock = luaM_reallocv(L, block, size, newsize, sizeof(T));
    size = newsize;  /* update only when everything else is OK */
    return newblock;
  }

  static T* luaM_realloc_(lua_State *L, T* block, size_t oldsize, size_t size)
  {
    T* newblock;
    size_t realosize = (block) ? oldsize : 0;
    lua_assert((realosize == 0) == (block == NULL));

    // TODO: remove
  #if defined(HARDMEMTESTS)
    //if (nsize > realosize && g->gcrunning)
    //  luaC_fullgc(L, 1);  /* force a GC whenever possible */
  #endif

    newblock = LuaAllocator<T>::alloc(block, oldsize, size);
    if (newblock == nullptr && size > 0)
      luaM_allocFail(L);

    lua_assert((size == 0) == (newblock == NULL));
    luaM_addGCDebt(L, size, realosize);

    return newblock;
  }
};
