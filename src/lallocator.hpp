#pragma once

template<class T>
class LuaAllocator
{
public:
  static T* alloc(T* ptr, size_t osize, size_t nsize)
  {
    (void)osize;  /* not used */
    if (nsize == 0)
    {
      LuaAllocator::free(ptr);
      return nullptr;
    }

    return static_cast<T*>(::realloc(ptr, nsize));
  }

  static void free(T* ptr)
  {
    ::free(ptr);
  }
};
