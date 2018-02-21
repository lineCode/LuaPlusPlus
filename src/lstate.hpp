#pragma once
/*
** $Id: lstate.h,v 2.133 2016/12/22 13:08:50 roberto Exp $
** Global State
** See Copyright Notice in lua.h
*/

#include <lua.hpp>
#include <lobject.hpp>
#include <ltm.hpp>
#include <lzio.hpp>
#include <array>

/*

 ** Some notes about garbage-collected objects: All objects in Lua must
 ** be kept somehow accessible until being freed, so all objects always
 ** belong to one (and only one) of these lists, using field 'next' of
 ** the 'CommonHeader' for the link:
 **
 ** 'allgc': all objects not marked for finalization;
 ** 'finobj': all objects marked for finalization;
 ** 'tobefnz': all objects ready to be finalized;
 ** 'fixedgc': all objects that are not to be collected (currently
 ** only small strings, such as reserved words).

 */

/*
** Atomic type (relative to signals) to better ensure that 'lua_sethook'
** is thread safe
*/
#if !defined(l_signalT)
#include <csignal>
#define l_signalT       sig_atomic_t
#endif

/* extra stack space to handle TM calls and some other extras */
#define EXTRA_STACK   5

#define BASIC_STACK_SIZE        (2*LUA_MINSTACK)

/* kinds of Garbage Collection */
#define KGC_NORMAL      0
#define KGC_EMERGENCY   1       /* gc was forced by an allocation failure */

/*
** Information about a call.
** When a thread yields, 'func' is adjusted to pretend that the
** top function has only the yielded values in its stack; in that
** case, the actual 'func' value is saved in field 'extra'.
** When a function calls another with a continuation, 'extra' keeps
** the function index so that, in case of errors, the continuation
** function can be called with the correct top.
*/
struct CallInfo
{
  StkId func;  /* function index in the stack */
  StkId top;  /* top for this function */
  struct CallInfo *previous, *next;  /* dynamic call link */
  union
  {
    struct
    {  /* only for Lua functions */
      StkId base;  /* base for this function */
      const Instruction *savedpc;
    } l;
    struct
    {  /* only for C functions */
      lua_KFunction k;  /* continuation in case of yields */
      ptrdiff_t old_errfunc;
      lua_KContext ctx;  /* context info. in case of yields */
    } c;
  } u;
  ptrdiff_t extra;
  short nresults;  /* expected number of results from this function */
  unsigned short callstatus;
};

/*
** Bits in CallInfo status
*/
#define CIST_OAH        (1<<0)  /* original value of 'allowhook' */
#define CIST_LUA        (1<<1)  /* call is running a Lua function */
#define CIST_HOOKED     (1<<2)  /* call is running a debug hook */
#define CIST_FRESH      (1<<3)  /* call is running on a fresh invocation
                                   of luaV_execute */
#define CIST_YPCALL     (1<<4)  /* call is a yieldable protected call */
#define CIST_TAIL       (1<<5)  /* call was tail called */
#define CIST_HOOKYIELD  (1<<6)  /* last hook called yielded */
#define CIST_LEQ        (1<<7)  /* using __lt for __le */
#define CIST_FIN        (1<<8)  /* call is running a finalizer */

#define isLua(ci)       ((ci)->callstatus & CIST_LUA)

/* assume that CIST_OAH has offset 0 and that 'v' is strictly 0/1 */
#define setoah(st, v)    ((st) = ((st) & ~CIST_OAH) | (v))
#define getoah(st)      ((st) & CIST_OAH)

struct Stringtable
{
  TString** hash = nullptr;
  int nuse = 0;  /* number of elements */
  int size = 0;
};

/*
** 'global state', shared by all threads of this state
*/
class global_State
{
public:
  global_State();
  ~global_State();

  /* actual number of total bytes allocated */
  lu_mem getTotalBytes() const { return this->totalbytes + this->GCdebt; }

  l_mem totalbytes = 0;  /* number of bytes currently allocated - GCdebt */
  l_mem GCdebt = 0;  /* bytes allocated not yet compensated by the collector */
  lu_mem GCmemtrav = 0;  /* memory traversed by the GC */
  lu_mem GCestimate = 0;  /* an estimate of the non-garbage memory in use */
  Stringtable strt;  /* hash table for strings */
  TValue l_registry;
  uint32_t seed = 0;  /* randomized seed for hashes */
  uint8_t currentwhite = 0;
  uint8_t gcstate = 0;  /* state of garbage collector */
  uint8_t gckind = 0;  /* kind of GC running */
  bool gcrunning = false;  /* true if GC is running */
  GCObject* allgc = nullptr;  /* list of all collectable objects */
  GCObject** sweepgc = nullptr;  /* current position of sweep in list */
  GCObject* finobj = nullptr;  /* list of collectable objects with finalizers */
  GCObject* gray = nullptr;  /* list of gray objects */
  GCObject* grayagain = nullptr;  /* list of objects to be traversed atomically */
  GCObject* weak = nullptr;  /* list of tables with weak values */
  GCObject* ephemeron = nullptr;  /* list of ephemeron tables (weak keys) */
  GCObject* allweak = nullptr;  /* list of all-weak tables */
  GCObject* tobefnz = nullptr;  /* list of userdata to be GC */
  GCObject* fixedgc = nullptr;  /* list of objects not to be collected */
  class lua_State* twups = nullptr;  /* list of threads with open upvalues */
  uint32_t gcfinnum = 0;  /* number of finalizers to call in each GC step */
  int gcpause = 0;  /* size of pause between successive GCs */
  int gcstepmul = 0;  /* GC 'granularity' */
  lua_CFunction panic = nullptr;  /* to be called in unprotected errors */
  class lua_State* mainthread = nullptr;
  const lua_Number* version = nullptr;  /* pointer to version number */
  TString* memerrmsg = nullptr;  /* memory-error message */
  std::array<TString*, TM_N> tmname {}; /* array with tag-method names */
  std::array<Table*, LuaType::basic_types_count> mt {}; /* metatables for basic types */
  std::array<std::array<TString*, STRCACHE_M>, STRCACHE_N> strcache {}; /* cache for strings in API */
};

struct lua_ErrorStatus;
/*
** 'per thread' state
*/
class lua_State : public GCObject
{
public:
  lua_State();
  explicit lua_State(lua_State* L);
  ~lua_State();

  unsigned short nci;  /* number of items in 'ci' list */
  uint8_t status;
  StkId top;  /* first free slot in the stack */
  global_State* globalState;
  CallInfo *ci;  /* call info for current function */
  const Instruction *oldpc;  /* last pc traced */
  StkId stack_last;  /* last free slot in the stack */
  StkId stack;  /* stack base */
  UpVal *openupval;  /* list of open upvalues in this stack */
  GCObject *gclist;
  lua_State* twups;  /* list of threads with open upvalues */
  struct lua_ErrorStatus* errorStatus;  /* current error data */
  CallInfo base_ci;  /* CallInfo for first level (C calling Lua) */
  volatile lua_Hook hook;
  ptrdiff_t errfunc;  /* current error handling function (stack index) */
  int stacksize;
  int basehookcount;
  int hookcount;
  unsigned short nny;  /* number of non-yieldable calls in stack */
  unsigned short nCcalls;  /* number of nested C calls */
  l_signalT hookmask;
  uint8_t allowhook;
};

/* macros to convert a GCObject into a specific value */
#define gco2ts(o) check_exp(novariant((o)->type) == LuaType::Basic::String, static_cast<TString*>(o))
#define gco2u(o)  check_exp((o)->type == LuaType::Variant::UserData, static_cast<Udata*>(o))
#define gco2lcl(o)  check_exp((o)->type == LuaType::Variant::LuaFunctionClosure, static_cast<LClosure*>(o))
#define gco2ccl(o)  check_exp((o)->type == LuaType::Variant::CFunctionClosure, static_cast<CClosure*>(o))
#define gco2t(o)  check_exp((o)->type == LuaType::Variant::Table, static_cast<Table*>(o))
#define gco2p(o)  check_exp((o)->type == LuaType::Variant::FunctionPrototype, static_cast<Proto*>(o))
#define gco2th(o)  check_exp((o)->type == LuaType::Variant::Thread, static_cast<lua_State*>(o))

/* macro to convert a Lua object into a GCObject */
#define obj2gco(v) \
  check_exp(LuaType::DataType(novariant((v)->type)) < LuaType::DataType(LuaType::Variant::DeadKey), (static_cast<GCObject*>(v)))

LUAI_FUNC void luaE_setdebt(global_State *g, l_mem debt);
LUAI_FUNC CallInfo *luaE_extendCI(lua_State *L);
LUAI_FUNC void luaE_freeCI(lua_State *L);
LUAI_FUNC void luaE_shrinkCI(lua_State *L);
LUAI_FUNC void freestack(lua_State* L);
