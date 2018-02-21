#pragma once
/*
** $Id: lcode.h,v 1.64 2016/01/05 16:22:37 roberto Exp $
** Code generator for Lua
** See Copyright Notice in lua.h
*/

#include <llex.hpp>
#include <lobject.hpp>
#include <lopcodes.hpp>
#include <lparser.hpp>

/*
** Marks the end of a patch list. It is an invalid value both as an absolute
** address, and as a list link (would link an element to itself).
*/
static constexpr int32_t NO_JUMP = -1;

/*
** grep "ORDER OPR" if you change these enums  (ORDER OP)
*/
enum BinOpr : uint8_t
{
  OPR_ADD, OPR_SUB, OPR_MUL, OPR_MOD, OPR_POW,
  OPR_DIV,
  OPR_IDIV,
  OPR_BAND, OPR_BOR, OPR_BXOR,
  OPR_SHL, OPR_SHR,
  OPR_CONCAT,
  OPR_EQ, OPR_LT, OPR_LE,
  OPR_NE, OPR_GT, OPR_GE,
  OPR_AND, OPR_OR,
  OPR_NOBINOPR
};

enum UnOpr : uint8_t
{
  OPR_MINUS,
  OPR_BNOT,
  OPR_NOT,
  OPR_LEN,
  OPR_NOUNOPR
};

/* get (pointer to) instruction of given 'expdesc' */
#define getinstruction(fs, e)    ((fs)->f->code[(e)->u.info])

#define luaK_codeAsBx(fs, o, A, sBx)       luaK_codeABx(fs, o, A, (sBx)+MAXARG_sBx)

#define luaK_setmultret(fs, e)   luaK_setreturns(fs, e, LUA_MULTRET)

#define luaK_jumpto(fs, t)       luaK_patchlist(fs, luaK_jump(fs), t)

LUAI_FUNC int luaK_codeABx(FuncState *fs, OpCode o, int A, uint32_t Bx);
LUAI_FUNC int luaK_codeABC(FuncState *fs, OpCode o, int A, int B, int C);
LUAI_FUNC int luaK_codek(FuncState *fs, int reg, int k);
LUAI_FUNC void luaK_fixline(FuncState *fs, int line);
LUAI_FUNC void luaK_nil(FuncState *fs, int from, int n);
LUAI_FUNC void luaK_reserveregs(FuncState *fs, int n);
LUAI_FUNC void luaK_checkstack(FuncState *fs, int n);
LUAI_FUNC int luaK_stringK(FuncState *fs, TString *s);
LUAI_FUNC int luaK_intK(FuncState *fs, lua_Integer n);
LUAI_FUNC void luaK_dischargevars(FuncState *fs, ExpressionDescription *e);
LUAI_FUNC int luaK_exp2anyreg(FuncState *fs, ExpressionDescription *e);
LUAI_FUNC void luaK_exp2anyregup(FuncState *fs, ExpressionDescription *e);
LUAI_FUNC void luaK_exp2nextreg(FuncState *fs, ExpressionDescription *e);
LUAI_FUNC void luaK_exp2val(FuncState *fs, ExpressionDescription *e);
LUAI_FUNC int luaK_exp2RK(FuncState *fs, ExpressionDescription *e);
LUAI_FUNC void luaK_self(FuncState *fs, ExpressionDescription *e, ExpressionDescription *key);
LUAI_FUNC void luaK_indexed(FuncState *fs, ExpressionDescription *t, ExpressionDescription *k);
LUAI_FUNC void luaK_goiftrue(FuncState *fs, ExpressionDescription *e);
LUAI_FUNC void luaK_goiffalse(FuncState *fs, ExpressionDescription *e);
LUAI_FUNC void luaK_storevar(FuncState *fs, ExpressionDescription *var, ExpressionDescription *e);
LUAI_FUNC void luaK_setreturns(FuncState *fs, ExpressionDescription *e, int nresults);
LUAI_FUNC void luaK_setoneret(FuncState *fs, ExpressionDescription *e);
LUAI_FUNC int luaK_jump(FuncState *fs);
LUAI_FUNC void luaK_ret(FuncState *fs, int first, int nret);
LUAI_FUNC void luaK_patchlist(FuncState *fs, int list, int target);
LUAI_FUNC void luaK_patchtohere(FuncState *fs, int list);
LUAI_FUNC void luaK_patchclose(FuncState *fs, int list, int level);
LUAI_FUNC void luaK_concat(FuncState *fs, int *l1, int l2);
LUAI_FUNC int luaK_getlabel(FuncState *fs);
LUAI_FUNC void luaK_prefix(FuncState *fs, UnOpr op, ExpressionDescription *v, int line);
LUAI_FUNC void luaK_infix(FuncState *fs, BinOpr op, ExpressionDescription *v);
LUAI_FUNC void luaK_posfix(FuncState *fs, BinOpr op, ExpressionDescription *v1,
                           ExpressionDescription *v2, int line);
LUAI_FUNC void luaK_setlist(FuncState *fs, int base, int nelems, int tostore);
