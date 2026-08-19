// This file contains utility functions that cast input values to different
// types and value categories.
#pragma once

#include "metafunctions/type_trait_mfns.hh"

namespace util {

// Forwards `value` with the value category that was originally passed at the
// callsite. This overload takes a plain lvalue reference.
//
// If `T` is an lvalue reference type, the return type collapses to lavlue
// reference (`T& &&`). If `T` is a non-reference type, the return type
// collapses to rvalue (`T &&`). If `T` is an rvalue reference type, the return
// type collapses to rvalue (`T&& &&`).
template <typename T>
constexpr T&& forward(metafunctions::remove_reference_t<T>& value) noexcept {
  return static_cast<T&&>(value);
}

// Forwards `value` as an rvalue. This overload only accepts rvalues.
template <typename T>
constexpr T&& forward(metafunctions::remove_reference_t<T>&& value) noexcept {
  return static_cast<T&&>(value);
}

// Casts the input lvalue to rvalue.
template <typename T>
constexpr metafunctions::remove_reference_t<T>&& move(T&& value) noexcept {
  return static_cast<metafunctions::remove_reference_t<T>&&>(value);
}

}  // namespace util
