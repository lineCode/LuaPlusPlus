/*
** $Id: lstate.c,v 2.133 2015/11/13 12:16:51 roberto Exp $
** Global State
** See Copyright Notice in lua.h
*/

#define lstate_c
#define LUA_CORE

#include <lprefix.hpp>

#include <cstddef>
#include <cstring>
#include <lua.hpp>
#include <lapi.hpp>
#include <ldebug.hpp>
#include <ldo.hpp>
#include <lfunc.hpp>
#include <lgc.hpp>
#include <llex.hpp>
#include <lmem.hpp>
#include <lstate.hpp>
#include <lstring.hpp>
#include <ltable.hpp>
#include <ltm.hpp>
#include <lauxlib.hpp>


#if !defined(LUAI_GCPAUSE)
#define LUAI_GCPAUSE	200  /* 200% */
#endif

#if !defined(LUAI_GCMUL)
#define LUAI_GCMUL	200 /* GC runs 'twice the speed' of memory allocation */
#endif


/*
** a macro to help the creation of a unique random seed when a state is
** created; the seed is used to randomize hashes.
*/
#if !defined(luai_makeseed)
#include <time.h>
#define luai_makeseed()		cast(uint32_t, time(NULL))
#endif

/*
** Compute an initial seed as random as possible. Rely on Address Space
** Layout Randomization (if present) to increase randomness..
*/

static uint32_t makeseed(lua_State* L)
{
  char buff[3 * sizeof(size_t)];
  uint32_t h = luai_makeseed();
  int p = 0;

  auto addBuff = [](auto& b, auto& p, const auto& e)
  {
    size_t t = reinterpret_cast<size_t>(e);
    memcpy(b + p, &t, sizeof(t));
    p += sizeof(t);
  };

  addBuff(buff, p, L);  /* heap variable */
  addBuff(buff, p, &h);  /* local variable */
  addBuff(buff, p, luaO_nilobject);  /* global variable */
  lua_assert(p == sizeof(buff));

  return luaS_hash(buff, p, h);
}

void freestack (lua_State *L)
{
  if (L->stack == nullptr)
    return;  /* stack not completely built yet */
  L->ci = &L->base_ci;  /* free the entire 'ci' list */
  luaE_freeCI(L);
  lua_assert(L->nci == 0);
  LMem<TValue>::luaM_freearray(L, L->stack, L->stacksize);  /* free stack array */
}


/*
** Create registry table and its predefined values
*/
static void init_registry (lua_State *L, global_State *g) {
  TValue temp;
  /* create registry */
  Table *registry = luaH_new(L);
  sethvalue(L, &g->l_registry, registry);
  luaH_resize(L, registry, LUA_RIDX_LAST, 0);
  /* registry[LUA_RIDX_MAINTHREAD] = L */
  setthvalue(L, &temp, L);  /* temp = L */
  luaH_setint(L, registry, LUA_RIDX_MAINTHREAD, &temp);
  /* registry[LUA_RIDX_GLOBALS] = table of globals */
  sethvalue(L, &temp, luaH_new(L));  /* temp = new table (global table) */
  luaH_setint(L, registry, LUA_RIDX_GLOBALS, &temp);
}

static void stack_init (lua_State *L1, lua_State *L)
{
  int i; CallInfo *ci;
  /* initialize stack array */
  L1->stack = LMem<TValue>::luaM_newvector(L, BASIC_STACK_SIZE);
  L1->stacksize = BASIC_STACK_SIZE;
  for (i = 0; i < BASIC_STACK_SIZE; i++)
    setnilvalue(L1->stack + i);  /* erase new stack */
  L1->top = L1->stack;
  L1->stack_last = L1->stack + L1->stacksize - EXTRA_STACK;
  /* initialize first ci */
  ci = &L1->base_ci;
  ci->next = ci->previous = NULL;
  ci->callstatus = 0;
  ci->func = L1->top;
  setnilvalue(L1->top++);  /* 'function' entry for this 'ci' */
  ci->top = L1->top + LUA_MINSTACK;
  L1->ci = ci;
}

/*
** open parts of the state that may cause memory-allocation errors.
** ('g->version' != NULL flags that the state was completely build)
*/
static void f_luaopen (lua_State *L, void *ud)
{
  global_State *g = L->globalState;
  UNUSED(ud);
  stack_init(L, L);  /* init stack */
  init_registry(L, g);
  luaS_init(L);
  luaT_init(L);
  luaX_init(L);
  g->gcrunning = true;  /* allow gc */
  g->version = lua_version(nullptr);
  luai_userstateopen(L);
}


/*
** preinitialize a thread with consistent values without allocating
** any memory (to avoid errors)
*/
static void preinit_thread (lua_State *L, global_State *g)
{
  L->globalState = g;
  L->stack = nullptr;
  L->ci = nullptr;
  L->nci = 0;
  L->stacksize = 0;
  L->twups = L;  /* thread has no upvalues */
  L->errorStatus = nullptr;
  L->nCcalls = 0;
  L->hook = nullptr;
  L->hookmask = 0;
  L->basehookcount = 0;
  L->allowhook = 1;
  resethookcount(L);
  L->openupval = nullptr;
  L->nny = 1;
  L->status = LUA_OK;
  L->errfunc = 0;
}

static int panic (lua_State *L)
{
  lua_writestringerror("PANIC: unprotected error in call to Lua API (%s)\n",
                        lua_tostring(L, -1));
  return 0;  /* return to Lua to abort */
}

global_State::global_State() = default;

global_State::~global_State() = default;

lua_State::lua_State()
{
  this->globalState = new global_State();

  global_State* g = this->globalState;
  this->next = nullptr;
  this->tt = LUA_TTHREAD;
  g->currentwhite = bitmask(WHITE0BIT);
  this->marked = luaC_white(g);
  preinit_thread(this, g);
  g->mainthread = this;
  g->seed = makeseed(this);
  setnilvalue(&g->l_registry);
  g->gcstate = GCSpause;
  g->gckind = KGC_NORMAL;
  g->totalbytes = sizeof(lua_State) + sizeof(global_State);
  g->gcpause = LUAI_GCPAUSE;
  g->gcstepmul = LUAI_GCMUL;
  f_luaopen(this, nullptr);
  lua_atpanic(this, &panic);
}

lua_State::lua_State(lua_State* L)
{
  global_State *g = L->globalState;
  lua_lock(L);
  luaC_checkGC(L);
  lua_State* L1 = this;

  // Add self to GC Debt
  luaM_addGCDebt(L, sizeof(lua_State), 0);

  L1->marked = luaC_white(g);
  L1->tt = LUA_TTHREAD;
  /* link it on list 'allgc' */
  L1->next = g->allgc;
  g->allgc = obj2gco(L1);
  /* anchor it on L stack */
  setthvalue(L, L->top, L1);
  api_incr_top(L);
  preinit_thread(L1, g);
  L1->hookmask = L->hookmask;
  L1->basehookcount = L->basehookcount;
  L1->hook = L->hook;
  resethookcount(L1);
  luai_userstatethread(L, L1);
  stack_init(L1, L);  /* init stack */
  lua_unlock(L);
}

lua_State::~lua_State()
{
  if (this->globalState->mainthread == this)
  {
    global_State *g = this->globalState;
    luaF_close(this, this->stack);  /* close all upvalues for this thread */
    luaC_freeallobjects(this);  /* collect all objects */
    if (g->version)  /* closing a fully built state? */
      luai_userstateclose(this);
    LMem<TString*>::luaM_freearray(this, this->globalState->strt.hash, this->globalState->strt.size);
    freestack(this);
    lua_assert(g->getTotalBytes() == sizeof(lua_State) + sizeof(global_State));

    delete this->globalState;
  }
  else
  {
    lua_State* L = LGCFactory::getActiveState();
    luaF_close(this, this->stack);  /* close all upvalues for this thread */
    lua_assert(this->openupval == NULL);
    luai_userstatefree(L, this);
    freestack(this);
    // Remove self from GC Debt
    luaM_addGCDebt(L, 0, sizeof(lua_State));
  }
}

/*
** set GCdebt to a new value keeping the value (totalbytes + GCdebt)
** invariant (and avoiding underflows in 'totalbytes')
*/
void luaE_setdebt (global_State *g, l_mem debt) {
  l_mem tb = g->getTotalBytes();
  lua_assert(tb > 0);
  if (debt < tb - MAX_LMEM)
    debt = tb - MAX_LMEM;  /* will make 'totalbytes == MAX_LMEM' */
  g->totalbytes = tb - debt;
  g->GCdebt = debt;
}


CallInfo *luaE_extendCI (lua_State *L) {
  CallInfo *ci = LMem<CallInfo>::luaM_new(L);
  lua_assert(L->ci->next == NULL);
  L->ci->next = ci;
  ci->previous = L->ci;
  ci->next = NULL;
  L->nci++;
  return ci;
}


/*
** free all CallInfo structures not in use by a thread
*/
void luaE_freeCI (lua_State *L) {
  CallInfo *ci = L->ci;
  CallInfo *next = ci->next;
  ci->next = NULL;
  while ((ci = next) != NULL) {
    next = ci->next;
    LMem<CallInfo>::luaM_free(L, ci);
    L->nci--;
  }
}


/*
** free half of the CallInfo structures not in use by a thread
*/
void luaE_shrinkCI (lua_State *L) {
  CallInfo *ci = L->ci;
  CallInfo *next2;  /* next's next */
  /* while there are two nexts */
  while (ci->next != NULL && (next2 = ci->next->next) != NULL) {
    LMem<CallInfo>::luaM_free(L, ci->next);  /* free next */
    L->nci--;
    ci->next = next2;  /* remove 'next' from the list */
    next2->previous = ci;
    ci = next2;  /* keep next's next */
  }
}

LUA_API lua_State *lua_newthread (lua_State *L)
{
  return new lua_State(L);
}
