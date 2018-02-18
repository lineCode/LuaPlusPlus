#pragma once

namespace Lua
{
  template<class T>
  class ScopedValueSetter
  {
  public:
    ScopedValueSetter(T& value, T newValue)
      : value(value)
      , originalValue(value)
    {
      this->value = newValue;
    }
    ~ScopedValueSetter()
    {
      this->value = this->originalValue;
    }
  private:
    T& value;
    T originalValue;
  };
}
