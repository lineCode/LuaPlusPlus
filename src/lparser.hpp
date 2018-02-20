#pragma once
/*
** $Id: lparser.h,v 1.76 2015/12/30 18:16:13 roberto Exp $
** Lua Parser
** See Copyright Notice in lua.h
*/

#include <llimits.hpp>
#include <lobject.hpp>
#include <lzio.hpp>


/*
** Expression and variable descriptor.
** Code generation for variables and expressions can be delayed to allow
** optimizations; An 'expdesc' structure describes a potentially-delayed
** variable/expression. It has a description of its "main" value plus a
** list of conditional jumps that can also produce its value (generated
** by short-circuit operators 'and'/'or').
*/

/* kinds of variables/expressions */
enum ExpType
{
  VVOID,  /* when 'expdesc' describes the last expression a list,
            this kind means an empty list (so, no expression) */
  VNIL,  /* constant nil */
  VTRUE,  /* constant true */
  VFALSE,  /* constant false */
  VK,  /* constant in 'k'; info = index of constant in 'k' */
  VKFLT,  /* floating constant; nval = numerical float value */
  VKINT,  /* integer constant; nval = numerical integer value */
  VNONRELOC,  /* expression has its value in a fixed register;
                info = result register */
  VLOCAL,  /* local variable; info = local register */
  VUPVAL,  /* upvalue variable; info = index of upvalue in 'upvalues' */
  VINDEXED,  /* indexed variable;
               ind.vt = whether 't' is register or upvalue;
               ind.t = table register or upvalue;
               ind.idx = key's R/K index */
  VJMP,  /* expression is a test/comparison;
           info = pc of corresponding jump instruction */
  VRELOCABLE,  /* expression can put result in any register;
                 info = instruction pc */
  VCALL,  /* expression is a function call; info = instruction pc */
  VVARARG  /* vararg expression; info = instruction pc */
};


#define vkisvar(k)	(VLOCAL <= (k) && (k) <= VINDEXED)
#define vkisinreg(k)	((k) == VNONRELOC || (k) == VLOCAL)

struct ExpressionDescription
{
  ExpType k;
  union {
    lua_Integer ival;    /* for VKINT */
    lua_Number nval;  /* for VKFLT */
    int info;  /* for generic use */
    struct {  /* for indexed variables (VINDEXED) */
      short idx;  /* index (R/K) */
      uint8_t t;  /* table (register or upvalue) */
      uint8_t vt;  /* whether 't' is register (VLOCAL) or upvalue (VUPVAL) */
    } ind;
  } u;
  int t;  /* patch list of 'exit when true' */
  int f;  /* patch list of 'exit when false' */
};


/* description of active local variable */
typedef struct Vardesc {
  short idx;  /* variable index in stack */
} Vardesc;


/* description of pending goto statements and label statements */
struct LabelDescription
{
  TString *name;  /* label identifier */
  int pc;  /* position in code */
  int line;  /* line where it appeared */
  uint8_t nactvar;  /* local level where it appears in current block */
};


/* list of labels or gotos */
struct Labellist
{
  LabelDescription* arr = nullptr;  /* array */
  int n = 0;  /* number of entries in use */
  int size = 0;  /* array size */
};


/* dynamic structures used by the parser */
struct Dyndata
{
  struct
  { /* list of active local variables */
    Vardesc* arr = nullptr;
    int n = 0;
    int size = 0;
  } actvar;
  Labellist gt;  /* list of pending gotos */
  Labellist label;   /* list of active labels */
};


/* control of blocks */
struct BlockCnt;  /* defined in lparser.c */


/* state needed to generate code for a given function */
typedef struct FuncState {
  Proto *f;  /* current function header */
  struct FuncState *prev;  /* enclosing function */
  struct LexState *ls;  /* lexical state */
  struct BlockCnt *bl;  /* chain of current blocks */
  int pc;  /* next position to code (equivalent to 'ncode') */
  int lasttarget;   /* 'label' of last 'jump label' */
  int jpc;  /* list of pending jumps to 'pc' */
  int nk;  /* number of elements in 'k' */
  int np;  /* number of elements in 'p' */
  int firstlocal;  /* index of first local var (in Dyndata array) */
  short nlocvars;  /* number of elements in 'f->locvars' */
  uint8_t nactvar;  /* number of active local variables */
  uint8_t nups;  /* number of upvalues */
  uint8_t freereg;  /* first free register */
} FuncState;


LUAI_FUNC LClosure *luaY_parser (lua_State *L, ZIO& z, Mbuffer& buff,
                                 Dyndata& dyd, const char *name, int firstchar);
