// This file contains identity trait metafunctions and corresponding
// transformation alias templates.
#pragma once

#include <concepts>

namespace metafunctions {

// Type identity trait for retrieval via `::type`.
template <typename T>
struct type_identity {
  using type = T;
};

// Identity trait for integral constants.
template <typename T, T Value>
  requires std::integral<T>
struct integral_constant {
  static constexpr T value = Value;

  using value_type = T;
  using type = integral_constant;
};

// Identity trait for boolean constants.
template <bool Value>
using bool_constant = integral_constant<bool, Value>;

// Identity trait for a true statement in TMP.
using true_type = bool_constant<true>;
// Identity trait for a false statement in TMP.
using false_type = bool_constant<false>;

// Identity trait used to indicate a valid expressions or member types.
template <typename... Args>
using void_t = void;

}  // namespace metafunctions
