#pragma once
/*
** $Id: luaconf.h,v 1.259 2016/12/22 13:08:50 roberto Exp $
** Configuration file for Lua
** See Copyright Notice in lua.h
*/

#include <cstdint>
#include <climits>
#include <limits>

/*
** ===================================================================
** Search for "@@" to find all configurable definitions.
** ===================================================================
*/

/*
** {====================================================================
** System Configuration: macros to adapt (if needed) Lua to some
** particular platform, for instance compiling it with 32-bit numbers or
** restricting it to C89.
** =====================================================================
*/

/*
   @@ LUA_USE_C89 controls the use of non-ISO-C89 features.
 ** Define it if you want Lua to avoid the use of a few C99 features
 ** or Windows-specific features on Windows.
 */
/* #define LUA_USE_C89 */

/*
** By default, Lua on Windows use (some) specific Windows features
*/
#if !defined(LUA_USE_C89) && defined(_WIN32) && !defined(_WIN32_WCE)
#define LUA_USE_WINDOWS  /* enable goodies for regular Windows */
#endif

#if defined(LUA_USE_WINDOWS)
#define LUA_DL_DLL      /* enable support for DLL */
#define LUA_USE_C89     /* broadly, Windows is C89 */
#endif

#if defined(LUA_USE_LINUX)
#define LUA_USE_POSIX
#define LUA_USE_DLOPEN          /* needs an extra library: -ldl */
#define LUA_USE_READLINE        /* needs some extra libraries */
#endif

#if defined(LUA_USE_MACOSX)
#define LUA_USE_POSIX
#define LUA_USE_DLOPEN          /* MacOS does not need -ldl */
#define LUA_USE_READLINE        /* needs an extra library: -lreadline */
#endif

/*
   @@ LUA_C89_NUMBERS ensures that Lua uses the largest types available for
 ** C89 ('long' and 'double'); Windows always has '__int64', so it does
 ** not need to use this case.
 */
#if defined(LUA_USE_C89) && !defined(LUA_USE_WINDOWS)
#define LUA_C89_NUMBERS
#endif

/*
** {==================================================================
** Configuration for Paths.
** ===================================================================
*/

/*
** LUA_PATH_SEP is the character that separates templates in a path.
** LUA_PATH_MARK is the string that marks the substitution points in a
** template.
** LUA_EXEC_DIR in a Windows path is replaced by the executable's
** directory.
*/
#define LUA_PATH_SEP            ";"
#define LUA_PATH_MARK           "?"
#define LUA_EXEC_DIR            "!"

/*
   @@ LUA_PATH_DEFAULT is the default path that Lua uses to look for
 ** Lua libraries.
   @@ LUA_CPATH_DEFAULT is the default path that Lua uses to look for
 ** C libraries.
 ** CHANGE them if your machine has a non-conventional directory
 ** hierarchy or if you want to install your libraries in
 ** non-conventional directories.
 */
#define LUA_VDIR        LUA_VERSION_MAJOR "." LUA_VERSION_MINOR
#if defined(_WIN32)     /* { */
/*
** In Windows, any exclamation mark ('!') in the path is replaced by the
** path of the directory of the executable file of the current process.
*/
#define LUA_LDIR        "!\\lua\\"
#define LUA_CDIR        "!\\"
#define LUA_SHRDIR      "!\\..\\share\\lua\\" LUA_VDIR "\\"
#define LUA_PATH_DEFAULT  \
  LUA_LDIR "?.lua;"  LUA_LDIR "?\\init.lua;" \
  LUA_CDIR "?.lua;"  LUA_CDIR "?\\init.lua;" \
  LUA_SHRDIR "?.lua;" LUA_SHRDIR "?\\init.lua;" \
  ".\\?.lua;" ".\\?\\init.lua"
#define LUA_CPATH_DEFAULT \
  LUA_CDIR "?.dll;" \
  LUA_CDIR "..\\lib\\lua\\" LUA_VDIR "\\?.dll;" \
  LUA_CDIR "loadall.dll;" ".\\?.dll"

#else                   /* }{ */

#define LUA_ROOT        "/usr/local/"
#define LUA_LDIR        LUA_ROOT "share/lua/" LUA_VDIR "/"
#define LUA_CDIR        LUA_ROOT "lib/lua/" LUA_VDIR "/"
#define LUA_PATH_DEFAULT  \
  LUA_LDIR "?.lua;"  LUA_LDIR "?/init.lua;" \
  LUA_CDIR "?.lua;"  LUA_CDIR "?/init.lua;" \
  "./?.lua;" "./?/init.lua"
#define LUA_CPATH_DEFAULT \
  LUA_CDIR "?.so;" LUA_CDIR "loadall.so;" "./?.so"
#endif                  /* } */

/*
   @@ LUA_DIRSEP is the directory separator (for submodules).
 ** CHANGE it if your machine does not use "/" as the directory separator
 ** and is not Windows. (On Windows Lua automatically uses "\".)
 */
#if defined(_WIN32)
#define LUA_DIRSEP      "\\"
#else
#define LUA_DIRSEP      "/"
#endif

/* }================================================================== */

/*
** {==================================================================
** Marks for exported symbols in the C code
** ===================================================================
*/

/*
   @@ LUA_API is a mark for all core API functions.
   @@ LUALIB_API is a mark for all auxiliary library functions.
   @@ LUAMOD_API is a mark for all standard library opening functions.
 ** CHANGE them if you need to define those functions in some special way.
 ** For instance, if you want to create one Windows DLL with the core and
 ** the libraries, you may want to use the following definition (define
 ** LUA_BUILD_AS_DLL to get it).
 */
#if defined(LUA_BUILD_AS_DLL)   /* { */

#if defined(LUA_CORE) || defined(LUA_LIB)       /* { */
#define LUA_API __declspec(dllexport)
#else                                           /* }{ */
#define LUA_API __declspec(dllimport)
#endif                                          /* } */

#else                           /* }{ */

#define LUA_API         extern

#endif                          /* } */

/* more often than not the libs go together with the core */
#define LUALIB_API      LUA_API
#define LUAMOD_API      LUALIB_API

/*
   @@ LUAI_FUNC is a mark for all extern functions that are not to be
 ** exported to outside modules.
   @@ LUAI_DDEF and LUAI_DDEC are marks for all extern (const) variables
 ** that are not to be exported to outside modules (LUAI_DDEF for
 ** definitions and LUAI_DDEC for declarations).
 ** CHANGE them if you need to mark them in some special way. Elf/gcc
 ** (versions 3.2 and later) mark them as "hidden" to optimize access
 ** when Lua is compiled as a shared library. Not all elf targets support
 ** this attribute. Unfortunately, gcc does not offer a way to check
 ** whether the target offers that support, and those without support
 ** give a warning about it. To avoid these warnings, change to the
 ** default definition.
 */
#if defined(__GNUC__) && ((__GNUC__*100 + __GNUC_MINOR__) >= 302) && \
  defined(__ELF__)            /* { */
#define LUAI_FUNC       __attribute__((visibility("hidden"))) extern
#else                           /* }{ */
#define LUAI_FUNC       extern
#endif                          /* } */

#define LUAI_DDEC       LUAI_FUNC
#define LUAI_DDEF       /* empty */

/* }================================================================== */

/*
** {==================================================================
** Configuration for Numbers.
** Change these definitions if no predefined LUA_FLOAT_* / LUA_INT_*
** satisfy your needs.
** ===================================================================
*/

/*
   @@ LUA_NUMBER is the floating-point type used by Lua.
   @@ over a floating number.
   @@ l_mathlim(x) corrects limit name 'x' to the proper float type
 ** by prefixing it with one of FLT/DBL/LDBL.
   @@ LUA_NUMBER_FRMLEN is the length modifier for writing floats.
   @@ LUA_NUMBER_FMT is the format for writing floats.
   @@ lua_number2str converts a float to a string.
   @@ l_mathop allows the addition of an 'l' or 'f' to all math operations.
   @@ l_floor takes the floor of a float.
   @@ lua_str2number converts a decimal numeric string to a number.
 */

/* The following definitions are good for most cases here */

#define l_floor(x)              (l_mathop(floor)(x))

#define lua_number2str(s, sz, n)  \
  l_sprintf((s), sz, LUA_NUMBER_FMT, (LUA_NUMBER)(n))

/*
   @@ lua_numbertointeger converts a float number to an integer, or
 ** returns 0 if float is not within the range of a lua_Integer.
 ** (The range comparisons are tricky because of rounding. The tests
 ** here assume a two-complement representation, where MININTEGER always
 ** has an exact representation as a float; MAXINTEGER may not have one,
 ** and therefore its conversion to float may have an ill-defined value.)
 */
#define lua_numbertointeger(n, p) \
  ((n) >= (LUA_NUMBER)(LUA_MININTEGER) && \
   (n) < -(LUA_NUMBER)(LUA_MININTEGER) && \
   (*(p) = (LUA_INTEGER)(n), 1))

/* now the variable definitions */

#define LUA_NUMBER      double

#define l_mathlim(n)            (DBL_ ## n)

#define LUA_NUMBER_FRMLEN       ""
#define LUA_NUMBER_FMT          "%.14g"

#define l_mathop(op)            op

#define lua_str2number(s, p)     strtod((s), (p))

/*
   @@ LUA_INTEGER is the integer type used by Lua.
 **
   @@ LUA_UNSIGNED is the unsigned version of LUA_INTEGER.
 **
   @@ over a lUA_INTEGER.
   @@ LUA_INTEGER_FRMLEN is the length modifier for reading/writing integers.
   @@ LUA_INTEGER_FMT is the format for writing integers.
   @@ LUA_MAXINTEGER is the maximum value for a LUA_INTEGER.
   @@ LUA_MININTEGER is the minimum value for a LUA_INTEGER.
   @@ lua_integer2str converts an integer to a string.
 */

/* use presence of macro LLONG_MAX as proxy for C99 compliance */
#if defined(LLONG_MAX)
/* use ISO C99 stuff */

using LUA_INTEGER = long long;
using LUA_UNSIGNED = unsigned long long;
#define LUA_INTEGER_FRMLEN      "ll"

// TODO: these should be constexpr variables but doing so triggers (I believe) undefined behavior and brakes parts of the Lua logic
// Look into it and fix what ever check is broken (there is more than one place in the code that does max + something which is guarenteed to be undefined behavior)
//static constexpr LUA_INTEGER LUA_MAXINTEGER = std::numeric_limits<LUA_INTEGER>::max();
//static constexpr LUA_INTEGER LUA_MININTEGER = std::numeric_limits<LUA_INTEGER>::min();
#define LUA_MAXINTEGER          LLONG_MAX
#define LUA_MININTEGER          LLONG_MIN

#else

#error "Compiler does not support 'long long'. Use option '-DLUA_C89_NUMBERS' (see file 'luaconf.h' for details)"

#endif

#define LUA_INTEGER_FMT         "%" LUA_INTEGER_FRMLEN "d"

#define lua_integer2str(s, sz, n)  \
  l_sprintf((s), sz, LUA_INTEGER_FMT, (LUA_INTEGER)(n))

/*
** {==================================================================
** Dependencies with C99 and other C details
** ===================================================================
*/

/*
   @@ l_sprintf is equivalent to 'snprintf' or 'sprintf' in C89.
 ** (All uses in Lua have only one format item.)
 */
#if !defined(LUA_USE_C89)
#define l_sprintf(s, sz, f, i)     snprintf(s, sz, f, i)
#else
#define l_sprintf(s, sz, f, i)     ((void)(sz), sprintf(s, f, i))
#endif

/*
   @@ lua_strx2number converts an hexadecimal numeric string to a number.
 ** In C99, 'strtod' does that conversion. Otherwise, you can
 ** leave 'lua_strx2number' undefined and Lua will provide its own
 ** implementation.
 */
#if !defined(LUA_USE_C89)
#define lua_strx2number(s, p)            lua_str2number(s, p)
#endif

/*
   @@ lua_number2strx converts a float to an hexadecimal numeric string.
 ** In C99, 'sprintf' (with format specifiers '%a'/'%A') does that.
 ** Otherwise, you can leave 'lua_number2strx' undefined and Lua will
 ** provide its own implementation.
 */
#if !defined(LUA_USE_C89)
#define lua_number2strx(L, b, sz, f, n)  \
  ((void)L, l_sprintf(b, sz, f, (LUA_NUMBER)(n)))
#endif

/*
** 'strtof' and 'opf' variants for math functions are not valid in
** C89. Otherwise, the macro 'HUGE_VALF' is a good proxy for testing the
** availability of these variants. ('math.h' is already included in
** all files that use these macros.)
*/
#if defined(LUA_USE_C89) || (defined(HUGE_VAL) && !defined(HUGE_VALF))
#undef l_mathop  /* variants not available */
#undef lua_str2number
#define l_mathop(op)            (lua_Number)op  /* no variant */
#define lua_str2number(s, p)     ((lua_Number)strtod((s), (p)))
#endif

/*
   @@ LUA_KCONTEXT is the type of the context ('ctx') for continuation
 ** functions.  It must be a numerical type; Lua will use 'intptr_t' if
 ** available, otherwise it will use 'ptrdiff_t' (the nearest thing to
 ** 'intptr_t' in C89)
 */
#define LUA_KCONTEXT    ptrdiff_t

#if !defined(LUA_USE_C89) && defined(__STDC_VERSION__) && \
  __STDC_VERSION__ >= 199901L
#include <stdint.h>
#if defined(INTPTR_MAX)  /* even in C99 this type is optional */
#undef LUA_KCONTEXT
#define LUA_KCONTEXT    intptr_t
#endif
#endif

/*
   @@ lua_getlocaledecpoint gets the locale "radix character" (decimal point).
 ** Change that if you do not want to use C locales. (Code using this
 ** macro must include header 'locale.h'.)
 */
#if !defined(lua_getlocaledecpoint)
#define lua_getlocaledecpoint()         (localeconv()->decimal_point[0])
#endif

/* }================================================================== */

/*
** {==================================================================
** Language Variations
** =====================================================================
*/

/*
   @@ LUA_NOCVTN2S/LUA_NOCVTS2N control how Lua performs some
 ** coercions. Define LUA_NOCVTN2S to turn off automatic coercion from
 ** numbers to strings. Define LUA_NOCVTS2N to turn off automatic
 ** coercion from strings to numbers.
 */
/* #define LUA_NOCVTN2S */
/* #define LUA_NOCVTS2N */

/*
   @@ LUA_USE_APICHECK turns on several consistency checks on the C API.
 ** Define it as a help when debugging C code.
 */
#if defined(LUA_USE_APICHECK)
#include <assert.h>
#define luai_apicheck(l, e)      assert(e)
#endif

/* }================================================================== */

/*
** {==================================================================
** Macros that affect the API and must be stable (that is, must be the
** same when you compile Lua and when you compile code that links to
** Lua). You probably do not want/need to change them.
** =====================================================================
*/

/*
   @@ LUAI_MAXSTACK limits the size of the Lua stack.
 ** CHANGE it if you need a different limit. This limit is arbitrary;
 ** its only purpose is to stop Lua from consuming unlimited stack
 ** space (and to reserve some numbers for pseudo-indices).
 */
#define LUAI_MAXSTACK           1000000

/*
   @@ LUA_IDSIZE gives the maximum size for the description of the source
   @@ of a function in debug information.
 ** CHANGE it if you want a different size.
 */
#define LUA_IDSIZE      60

/*
   @@ LUAL_BUFFERSIZE is the buffer size used by the lauxlib buffer system.
 ** CHANGE it if it uses too much C-stack space. (For long double,
 ** 'string.format("%.99f", -1e4932)' needs 5034 bytes, so a
 ** smaller buffer would force a memory allocation for each call to
 ** 'string.format'.)
 */
#define LUAL_BUFFERSIZE   ((int)(0x80 * sizeof(void*) * sizeof(lua_Integer)))

/* }================================================================== */

/*
   @@ LUA_QL describes how error messages quote program elements.
 ** Lua does not use these macros anymore; they are here for
 ** compatibility only.
 */
#define LUA_QL(x)       "'" x "'"
#define LUA_QS          LUA_QL("%s")
