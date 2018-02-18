/*
** $Id: lstring.c,v 2.56 2015/11/23 11:32:51 roberto Exp $
** String table (keeps all strings handled by Lua)
** See Copyright Notice in lua.h
*/

#define lstring_c
#define LUA_CORE

#include <lprefix.hpp>


#include <cstring>

#include <lua.hpp>

#include <ldebug.hpp>
#include <ldo.hpp>
#include <lmem.hpp>
#include <lobject.hpp>
#include <lstate.hpp>
#include <lstring.hpp>


#define MEMERRMSG       "not enough memory"


/*
** Lua will use at most ~(2^LUAI_HASHLIMIT) bytes from a string to
** compute its hash
*/
#if !defined(LUAI_HASHLIMIT)
#define LUAI_HASHLIMIT		5
#endif


/*
** equality for long strings
*/
int luaS_eqlngstr (TString *a, TString *b) {
  size_t len = a->u.lnglen;
  lua_assert(a->tt == LUA_TLNGSTR && b->tt == LUA_TLNGSTR);
  return (a == b) ||  /* same instance or... */
    ((len == b->u.lnglen) &&  /* equal length and ... */
     (memcmp(getstr(a), getstr(b), len) == 0));  /* equal contents */
}


uint32_t luaS_hash (const char *str, size_t l, uint32_t seed) {
  uint32_t h = seed ^ cast(uint32_t, l);
  size_t step = (l >> LUAI_HASHLIMIT) + 1;
  for (; l >= step; l -= step)
    h ^= ((h<<5) + (h>>2) + cast_byte(str[l - 1]));
  return h;
}


uint32_t luaS_hashlongstr (TString *ts) {
  lua_assert(ts->tt == LUA_TLNGSTR);
  if (ts->extra == 0) {  /* no hash? */
    ts->hash = luaS_hash(getstr(ts), ts->u.lnglen, ts->hash);
    ts->extra = 1;  /* now it has its hash */
  }
  return ts->hash;
}


/*
** resizes the string table
*/
void luaS_resize (lua_State *L, int newsize) {
  int i;
  stringtable *tb = &L->globalState->strt;
  if (newsize > tb->size) {  /* grow table if needed */
    LMem<TString*>::luaM_reallocvector(L, tb->hash, tb->size, newsize);
    for (i = tb->size; i < newsize; i++)
      tb->hash[i] = NULL;
  }
  for (i = 0; i < tb->size; i++) {  /* rehash */
    TString *p = tb->hash[i];
    tb->hash[i] = NULL;
    while (p) {  /* for each node in the list */
      TString *hnext = p->u.hnext;  /* save next */
      uint32_t h = lmod(p->hash, newsize);  /* new position */
      p->u.hnext = tb->hash[h];  /* chain it */
      tb->hash[h] = p;
      p = hnext;
    }
  }
  if (newsize < tb->size) {  /* shrink table if needed */
    /* vanishing slice should be empty */
    lua_assert(tb->hash[newsize] == NULL && tb->hash[tb->size - 1] == NULL);
    LMem<TString*>::luaM_reallocvector(L, tb->hash, tb->size, newsize);
  }
  tb->size = newsize;
}


/*
** Clear API string cache. (Entries cannot be empty, so fill them with
** a non-collectable string.)
*/
void luaS_clearcache (global_State *g) {
  int i, j;
  for (i = 0; i < STRCACHE_N; i++)
    for (j = 0; j < STRCACHE_M; j++) {
    if (iswhite(g->strcache[i][j]))  /* will entry be collected? */
      g->strcache[i][j] = g->memerrmsg;  /* replace it with something fixed */
    }
}


/*
** Initialize the string table and the string cache
*/
void luaS_init (lua_State *L) {
  global_State *g = L->globalState;
  int i, j;
  luaS_resize(L, MINSTRTABSIZE);  /* initial size of string table */
  /* pre-create memory-error message */
  g->memerrmsg = luaS_newliteral(L, MEMERRMSG);
  luaC_fix(L, obj2gco(g->memerrmsg));  /* it should never be collected */
  for (i = 0; i < STRCACHE_N; i++)  /* fill cache with valid strings */
    for (j = 0; j < STRCACHE_M; j++)
      g->strcache[i][j] = g->memerrmsg;
}



/*
** creates a new string object
*/
static TString *createstrobj (lua_State *L, size_t l, int tag, uint32_t h) {
  TString* result = LGCFactory::luaC_newobj<TString>(L, tag, sizelstring(l) /* total size of TString object */ );
  result->hash = h;
  result->extra = 0;
  getstr(result)[l] = '\0';  /* ending 0 */
  return result;
}


TString *luaS_createlngstrobj (lua_State *L, size_t l) {
  TString *ts = createstrobj(L, l, LUA_TLNGSTR, L->globalState->seed);
  ts->u.lnglen = l;
  return ts;
}


void luaS_remove (lua_State *L, TString *ts) {
  stringtable *tb = &L->globalState->strt;
  TString **p = &tb->hash[lmod(ts->hash, tb->size)];
  while (*p != ts)  /* find previous element */
    p = &(*p)->u.hnext;
  *p = (*p)->u.hnext;  /* remove element from its list */
  tb->nuse--;
}


/*
** checks whether short string exists and reuses it or creates a new one
*/
static TString *internshrstr (lua_State *L, const char *str, size_t l) {
  TString *ts;
  global_State *g = L->globalState;
  uint32_t h = luaS_hash(str, l, g->seed);
  TString **list = &g->strt.hash[lmod(h, g->strt.size)];
  lua_assert(str != NULL);  /* otherwise 'memcmp'/'memcpy' are undefined */
  for (ts = *list; ts != NULL; ts = ts->u.hnext) {
    if (l == ts->shrlen &&
        (memcmp(str, getstr(ts), l * sizeof(char)) == 0)) {
      /* found! */
      if (isdead(g, ts))  /* dead (but not collected yet)? */
        changewhite(ts);  /* resurrect it */
      return ts;
    }
  }
  if (g->strt.nuse >= g->strt.size && g->strt.size <= MAX_INT/2) {
    luaS_resize(L, g->strt.size * 2);
    list = &g->strt.hash[lmod(h, g->strt.size)];  /* recompute with new size */
  }
  ts = createstrobj(L, l, LUA_TSHRSTR, h);
  memcpy(getstr(ts), str, l * sizeof(char));
  ts->shrlen = cast_byte(l);
  ts->u.hnext = *list;
  *list = ts;
  g->strt.nuse++;
  return ts;
}


/*
** new string (with explicit length)
*/
TString *luaS_newlstr (lua_State *L, const char *str, size_t l) {
  if (l <= LUAI_MAXSHORTLEN)  /* short string? */
    return internshrstr(L, str, l);
  else {
    TString *ts;
    if (l >= (MAX_SIZE - sizeof(TString))/sizeof(char))
      luaM_toobig(L);
    ts = luaS_createlngstrobj(L, l);
    memcpy(getstr(ts), str, l * sizeof(char));
    return ts;
  }
}


/*
** Create or reuse a zero-terminated string, first checking in the
** cache (using the string address as a key). The cache can contain
** only zero-terminated strings, so it is safe to use 'strcmp' to
** check hits.
*/
TString *luaS_new (lua_State *L, const char *str) {
  uint32_t i = point2uint(str) % STRCACHE_N;  /* hash */
  int j;
  TString **p = L->globalState->strcache[i];
  for (j = 0; j < STRCACHE_M; j++) {
    if (strcmp(str, getstr(p[j])) == 0)  /* hit? */
      return p[j];  /* that is it */
  }
  /* normal route */
  for (j = STRCACHE_M - 1; j > 0; j--)
    p[j] = p[j - 1];  /* move out last element */
  /* new element is first in the list */
  p[0] = luaS_newlstr(L, str, strlen(str));
  return p[0];
}


Udata* luaS_newudata (lua_State *L, size_t s)
{
  if (s > MAX_SIZE - sizeof(Udata))
    luaM_toobig(L);
  Udata* u = LGCFactory::luaC_newobj<Udata>(L, LUA_TUSERDATA, sizeludata(s));
  u->len = s;
  u->metatable = nullptr;
  setuservalue(L, u, luaO_nilobject);
  return u;
}
