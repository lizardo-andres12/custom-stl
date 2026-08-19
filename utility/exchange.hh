#pragma once

#include <type_traits>

#include "utility/type_casting.hh"

namespace util {

// Replaces the value of `obj` with the value of `new_value` and returns the old
// value of `obj`.
template <typename T, typename U = T>
[[nodiscard]] constexpr T exchange(T& obj, U&& new_value) noexcept(
    noexcept(std::is_nothrow_move_constructible_v<T> &&
             std::is_nothrow_assignable_v<T&, U>)) {
  T ret = move(obj);
  obj = forward<U>(new_value);
  return ret;
}

}  // namespace util
