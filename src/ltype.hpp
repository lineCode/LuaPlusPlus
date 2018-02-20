#pragma once
#include <cstdint>
#include <cstdlib>
#include <limits>
class lua_State;

class LuaType
{
public:
  using DataType = int8_t;
  enum class Basic : DataType
  {
    Nil = 0,
    Boolean = 1,
    LightUserData = 2,
    Number = 3,
    String = 4,
    Table = 5,
    Function = 6,
    UserData = 7,
    Thread  = 8
  };

  static constexpr DataType basic_types_count = 9;
  static constexpr DataType basic_bits = 0x0F; // The first 4 bits
  static constexpr DataType variant_bits_shift = 4;
  static constexpr DataType collectable_bit_shift = 6;

  // Basic types use bits 1-4
  // Variants use bits 5-6
  // Collectable uses bit 7
  // Bit 8 is the signed bit and is used by "Variant::None"
  static_assert(basic_types_count - 1 <= basic_bits);

  // Variant uses all bits except the one used by collectable (bit 7)
  enum class Variant : DataType
  {
    None = -128, // 8th bit
    Nil = DataType(Basic::Nil),
    Boolean = DataType(Basic::Boolean),
    LightUserData = DataType(Basic::LightUserData),
    Number = DataType(Basic::Number),
    String = DataType(Basic::String),
    Table = DataType(Basic::Table),
    Function = DataType(Basic::Function),
    UserData = DataType(Basic::UserData),
    Thread = DataType(Basic::Thread),

    // Extra tags for non-values
    FunctionPrototype = basic_types_count,
    DeadKey = basic_types_count + 1, // removed keys in tables

    // Variant tags for functions
    LuaFunctionClosure = DataType(Basic::Function) | (0 << variant_bits_shift), // TLCL
    LightCFunction = DataType(Basic::Function) | (1 << variant_bits_shift), // TLCF
    CFunctionClosure = DataType(Basic::Function) | (2 << variant_bits_shift), // TCCL

    // Variant tags for strings
    ShortString = DataType(Basic::String) | (0 << variant_bits_shift),
    LongString = DataType(Basic::String) | (1 << variant_bits_shift),

    // Variant tags for numbers
    FloatNumber = DataType(Basic::Number) | (0 << variant_bits_shift),
    IntNumber = DataType(Basic::Number) | (1 << variant_bits_shift),
  };

  static constexpr DataType collectable = 1 << collectable_bit_shift;

  LuaType() = default;
  constexpr LuaType(Basic value)
    : value(DataType(value))
  {}
  constexpr LuaType::LuaType(Variant value)
    : value(DataType(value))
  {}

  constexpr Basic asBasic() const { return Basic(this->value & basic_bits); }
  constexpr Variant asVariant() const { return Variant(this->value & ~collectable); }
  // Gets the type as Basic or aborts if any bit outside of Basic is set
  Basic asBasicStrict() const;
  // Gets the type as Variant or aborts if any bit outside of Variant is set
  Variant asVariantStrict() const;
  constexpr DataType asUnderlying() const { return this->value; }
  constexpr LuaType asCollectable() const { return LuaType(DataType(this->value | collectable)); }

  constexpr bool isCollectable() const { return (this->value & collectable) != 0; }
  constexpr void setCollectable() { this->value |= collectable; }

  // Direct check if this is the exact basic type (if this is a variant or collectable it will fail)
  constexpr bool operator==(Basic value) const { return this->value == DataType(value); }
  // Direct check if this is the exact variant type (if this is collectable it will fail)
  constexpr bool operator==(Variant value) const { return this->value == DataType(value); }
  constexpr bool operator==(LuaType other) const { return this->value == other.value; }

  constexpr bool operator!=(Basic value) const { return this->value != DataType(value); }
  constexpr bool operator!=(Variant value) const { return this->value != DataType(value); }
  constexpr bool operator!=(LuaType other) const { return this->value != other.value; }

  static const char* toString(Basic value);
  static const char* toString(Variant value);
  static LuaType fromRaw(lua_State* L, DataType value);

private:
  explicit constexpr LuaType(DataType value) : value(value) {}

  DataType value;
};

inline LuaType::Basic LuaType::asBasicStrict() const
{
  if (*this != this->asBasic())
    abort();
  return this->asBasic();
}

inline LuaType::Variant LuaType::asVariantStrict() const
{
  if (*this != this->asVariant())
    abort();
  return this->asVariant();
}

inline bool operator==(LuaType::Basic type, LuaType other)
{
  return other == type;
}

inline bool operator==(LuaType::Variant type, LuaType other)
{
  return other == type;
}

static_assert(sizeof(LuaType) <= 1);
