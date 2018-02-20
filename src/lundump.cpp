/*
** $Id: lundump.c,v 2.44 2015/11/02 16:09:30 roberto Exp $
** load precompiled Lua chunks
** See Copyright Notice in lua.h
*/

#define lundump_c
#define LUA_CORE

#include <lprefix.hpp>


#include <cstring>

#include <lua.hpp>
#include <ldebug.hpp>
#include <ldo.hpp>
#include <lfunc.hpp>
#include <lmem.hpp>
#include <lobject.hpp>
#include <lstring.hpp>
#include <lundump.hpp>
#include <lzio.hpp>
#include <lauxlib.hpp>


#if !defined(luai_verifycode)
#define luai_verifycode(L,b,f)  /* empty */
#endif


struct LoadState
{
  LoadState(lua_State* L, ZIO& z, const char* name)
    : L(L)
    , Z(z)
    , name(name)
  {}
  lua_State* L;
  ZIO& Z;
  const char* name;
};


static void error(LoadState& S, const char* why)
{
  const char* what = luaO_pushfstring(S.L, "%s: %s precompiled chunk", S.name, why);
  luaD_throw(S.L, LUA_ERRSYNTAX, what);
}


/*
** All high-level loads go through LoadVector; you can change it to
** adapt to the endianness of the input
*/
#define LoadVector(S,b,n)	LoadBlock(S,b,(n)*sizeof((b)[0]))

static void LoadBlock (LoadState& S, void* b, size_t size)
{
  if (luaZ_read(S.Z, b, size) != 0)
    error(S, "truncated");
}


#define LoadVar(S,x)		LoadVector(S,&x,1)


static uint8_t LoadByte (LoadState& S) {
  uint8_t x;
  LoadVar(S, x);
  return x;
}


static int LoadInt (LoadState& S) {
  int x;
  LoadVar(S, x);
  return x;
}


static lua_Number LoadNumber (LoadState& S) {
  lua_Number x;
  LoadVar(S, x);
  return x;
}


static lua_Integer LoadInteger (LoadState& S)
{
  lua_Integer x;
  LoadVar(S, x);
  return x;
}


static TString *LoadString (LoadState& S)
{
  size_t size = LoadByte(S);
  if (size == 0xFF)
    LoadVar(S, size);
  if (size == 0)
    return nullptr;
  else if (--size <= LUAI_MAXSHORTLEN) {  /* short string? */
    char buff[LUAI_MAXSHORTLEN];
    LoadVector(S, buff, size);
    return luaS_newlstr(S.L, buff, size);
  }
  else {  /* long string */
    TString *ts = luaS_createlngstrobj(S.L, size);
    LoadVector(S, getstr(ts), size);  /* load directly in final place */
    return ts;
  }
}


static void LoadCode (LoadState& S, Proto *f)
{
  int n = LoadInt(S);
  f->code = LMem<Instruction>::luaM_newvector(S.L, n);
  f->sizecode = n;
  LoadVector(S, f->code, n);
}


static void LoadFunction(LoadState& S, Proto* f, TString* psource);


static void LoadConstants (LoadState& S, Proto* f)
{
  const int n = LoadInt(S);
  f->k = LMem<TValue>::luaM_newvector(S.L, n);
  f->sizek = n;
  for (int i = 0; i < n; i++)
    setnilvalue(&f->k[i]);
  for (int i = 0; i < n; i++)
  {
    TValue* o = &f->k[i];
    LuaType t = LuaType::fromRaw(S.L, LoadByte(S));
    switch (t.asVariantStrict())
    {
      case LuaType::Variant::Nil:
        setnilvalue(o);
        break;
      case LuaType::Variant::Boolean:
        setbvalue(o, LoadByte(S));
        break;
      case LuaType::Variant::FloatNumber:
        setfltvalue(o, LoadNumber(S));
        break;
      case LuaType::Variant::IntNumber:
        setivalue(o, LoadInteger(S));
        break;
      case LuaType::Variant::ShortString:
      case LuaType::Variant::LongString:
        setsvalue2n(S.L, o, LoadString(S));
        break;
      default:
        luaL_error(S.L, "Unknown type: %i.", t.asUnderlying());
    }
  }
}


static void LoadProtos (LoadState& S, Proto *f)
{
  int n = LoadInt(S);
  f->p = LMem<Proto*>::luaM_newvector(S.L, n);
  f->sizep = n;
  for (int i = 0; i < n; i++)
    f->p[i] = nullptr;
  for (int i = 0; i < n; i++)
  {
    f->p[i] = luaF_newproto(S.L);
    LoadFunction(S, f->p[i], f->source);
  }
}


static void LoadUpvalues (LoadState& S, Proto *f)
{
  int n = LoadInt(S);
  f->upvalues = LMem<Upvaldesc>::luaM_newvector(S.L, n);
  f->sizeupvalues = n;
  for (int i = 0; i < n; i++)
    f->upvalues[i].name = nullptr;
  for (int i = 0; i < n; i++)
  {
    f->upvalues[i].instack = LoadByte(S);
    f->upvalues[i].idx = LoadByte(S);
  }
}


static void LoadDebug (LoadState& S, Proto *f)
{
  int i, n;
  n = LoadInt(S);
  f->lineinfo = LMem<int>::luaM_newvector(S.L, n);
  f->sizelineinfo = n;
  LoadVector(S, f->lineinfo, n);
  n = LoadInt(S);
  f->locvars = LMem<LocVar>::luaM_newvector(S.L, n);
  f->sizelocvars = n;
  for (i = 0; i < n; i++)
    f->locvars[i].varname = nullptr;
  for (i = 0; i < n; i++) {
    f->locvars[i].varname = LoadString(S);
    f->locvars[i].startpc = LoadInt(S);
    f->locvars[i].endpc = LoadInt(S);
  }
  n = LoadInt(S);
  for (i = 0; i < n; i++)
    f->upvalues[i].name = LoadString(S);
}


static void LoadFunction (LoadState& S, Proto *f, TString *psource)
{
  f->source = LoadString(S);
  if (f->source == nullptr)  /* no source in dump? */
    f->source = psource;  /* reuse parent's source */
  f->linedefined = LoadInt(S);
  f->lastlinedefined = LoadInt(S);
  f->numparams = LoadByte(S);
  f->is_vararg = LoadByte(S);
  f->maxstacksize = LoadByte(S);
  LoadCode(S, f);
  LoadConstants(S, f);
  LoadUpvalues(S, f);
  LoadProtos(S, f);
  LoadDebug(S, f);
}


static void checkliteral (LoadState& S, const char *s, const char *msg) {
  char buff[sizeof(LUA_SIGNATURE) + sizeof(LUAC_DATA)]; /* larger than both */
  size_t len = strlen(s);
  LoadVector(S, buff, len);
  if (memcmp(s, buff, len) != 0)
    error(S, msg);
}


static void fchecksize (LoadState& S, size_t size, const char *tname) {
  if (LoadByte(S) != size)
    error(S, luaO_pushfstring(S.L, "%s size mismatch in", tname));
}


#define checksize(S,t)	fchecksize(S,sizeof(t),#t)

static void checkHeader (LoadState& S)
{
  checkliteral(S, LUA_SIGNATURE + 1, "not a");  /* 1st char already checked */
  if (LoadByte(S) != LUAC_VERSION)
    error(S, "version mismatch in");
  if (LoadByte(S) != LUAC_FORMAT)
    error(S, "format mismatch in");
  checkliteral(S, LUAC_DATA, "corrupted");
  checksize(S, int);
  checksize(S, size_t);
  checksize(S, Instruction);
  checksize(S, lua_Integer);
  checksize(S, lua_Number);
  if (LoadInteger(S) != LUAC_INT)
    error(S, "endianness mismatch in");
  if (LoadNumber(S) != LUAC_NUM)
    error(S, "float format mismatch in");
}


/*
** load precompiled chunk
*/
LClosure* luaU_undump(lua_State* L, ZIO& Z, const char* name)
{
  LClosure *cl;
  if (*name == '@' || *name == '=')
    name = name + 1;
  else if (*name == LUA_SIGNATURE[0])
    name = "binary string";
  LoadState S(L, Z, name);
  checkHeader(S);
  cl = luaF_newLclosure(L, LoadByte(S));
  setclLvalue(L, L->top, cl);
  luaD_inctop(L);
  cl->p = luaF_newproto(L);
  LoadFunction(S, cl->p, nullptr);
  lua_assert(cl->nupvalues == cl->p->sizeupvalues);
  luai_verifycode(L, buff, cl->p);
  return cl;
}

