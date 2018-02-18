/*
** $Id: lmem.c,v 1.91 2015/03/06 19:45:54 roberto Exp $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/

#define lmem_c
#define LUA_CORE

#include <lprefix.hpp>


#include <cstddef>

#include <lua.hpp>
#include <ldebug.hpp>
#include <ldo.hpp>
#include <lgc.hpp>
#include <lmem.hpp>
#include <lobject.hpp>
#include <lstate.hpp>



/*
** About the realloc function:
** void * frealloc (void *ud, void *ptr, size_t osize, size_t nsize);
** ('osize' is the old size, 'nsize' is the new size)
**
** * frealloc(ud, NULL, x, s) creates a new block of size 's' (no
** matter 'x').
**
** * frealloc(ud, p, x, 0) frees the block 'p'
** (in this specific case, frealloc must return NULL);
** particularly, frealloc(ud, NULL, 0, 0) does nothing
** (which is equivalent to free(NULL) in ISO C)
**
** frealloc returns NULL if it cannot create or reallocate the area
** (any reallocation to an equal or smaller size cannot fail!)
*/


void luaM_toobig (lua_State *L)
{
  luaG_runerror(L, "memory allocation error: block too big");
}

void luaM_addGCDebt(lua_State* L, size_t size, size_t realosize)
{
  global_State* g = L->globalState;
  g->GCdebt = (g->GCdebt + size) - realosize;
}

void luaM_allocFail(lua_State* L)
{
  luaD_throw(L, LUA_ERRMEM);
}

void luaM_tooMany(lua_State* L, const char* what, int32_t limit)
{
  luaG_runerror(L, "too many %s (limit is %d)", what, limit);
}
