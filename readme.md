# Lua++ basic info

This is my attempt at re-writing Lua 5.3 into modern (>= 14) C++.

A few basic things I decided just to keep my sanity intact:

* I'm doing this first-most for my own use - if it turns out to be usable by others that's great but it's not my main concern. I'm not going to go out of my way to make it unusable by others.
* I'm not going to make it support any Lua version before 5.3
* It will require your compiler be 32 bit or better
* It's always going to use "double" as the number type and "int64_t" as the integer type
* I probably won't keep the Lua 5.3 C API intact - at least not in a 1-1 form - but I haven't decided on that yet as I'm only a day into this as of writing this.
* There will be lots of trial-and-error here as I experiment with how I want to do everything so nothing is final.

My goals/general to-do in no specific order:

* Remove as many macros as possible.
  Reason: they make it incredibly hard to reason about what a piece of code is doing.
* Give things better names.
  Reason: the current name-scheme is trash: "uv->u", "uv->v" - almost everything has 1-3 character names and it makes it hard to tell what anything does.
* Use the boolean type instead of "int"
  Reason: "int" is not a boolean and takes up 3 more bytes compared to the boolean type.
* Get the code to compile under /WAll /Wx
  Reason: warnings almost always point at errors or at a minimum point at places where it's easy to make a mistake.
* Pack everything into structs/classes and use the private/public keywords instead of "extern" to hide functions.
  Reason: free-floating functions/constants pollute the namespace and do more harm than good.
* Avoid the use of primitive types in favor of fully defined types.
  Reason: "uint32_t" explains every aspect of what it is compared to "unsigned int"
* Drop support for a custom allocator and use global new/delete/make_unique.
  Reason: It clutters the code and gets in the way of reasoning about who owns memory.
* Make table iteration deterministic.
  Reason: It gets in the way of cross-platform stability and most importantly we need it to be deterministic in Factorio
* Make a C++ interface to replace the C interface
  Reason: The C interface is slow and doesn't allow for fine-tuned control.
* Experiment with basic serialize/de-serialize support: tables, numbers, strings, circular references.
  Reason: I want it for Factorio and it won't harm anything to exist if not used.
* Experiment with support to disable string interning.
  Reason: I think it does more harm than good but only testing will tell.