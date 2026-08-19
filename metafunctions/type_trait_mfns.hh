// This file contains metafunctions used to check conditions about types via
// template metaprogramming.
#pragma once

#include <cstddef>

#include "metafunctions/identity_trait_mfns.hh"
#include "metafunctions/util_mfns.hh"

namespace metafunctions {

// Template metafunction for checking if the type of `T` is void.
template <typename T>
struct is_void : false_type {};

// Template specialization for `void` types that inherits from `true_type`.
template <>
struct is_void<void> : true_type {};

// Template specialization for `const void` types that inherits from
// `true_type`.
template <>
struct is_void<const void> : true_type {};

// Template specialization for `volatile void` types that inherits from
// `true_type`.
template <>
struct is_void<volatile void> : true_type {};

// Template specialization for `const volatile void` types that inherits from
// `true_type`.
template <>
struct is_void<const volatile void> : true_type {};

// Transformation alias template that calls the `is_void` metafunction and
// returns the resulting value.
template <typename T>
static constexpr bool is_void_v = is_void<T>::value;

// Template metafunction for checking if types are the same at compile time.
// Default inherits `false_type`.
template <typename T, typename U>
struct is_same : false_type {};

// Template specialization of `is_same` for types that are the same. Inherits
// from `true_type`
template <typename T>
struct is_same<T, T> : true_type {};

// Transformation alias template that calls the `is_same` metafunction and
// returns the resulting value.
template <typename T, typename U>
static constexpr bool is_same_v = is_same<T, U>::value;

// Template metafunction that determines if `T` is an array type. Default
// inhetits from `false_type`.
template <typename T>
struct is_array : false_type {};

// Template metafunction specialization for unknown-length array types. Inherits
// from `true_type`.
template <typename T>
struct is_array<T[]> : true_type {};

// Template metafunction specialization for known-length array types. Inherits
// from `true_type`.
template <typename T, std::size_t N>
struct is_array<T[N]> : true_type {};

// Transformation alias template that calls the `is_array` metafunction and
// returns the resulting value.
template <typename T>
static constexpr bool is_array_v = is_array<T>::value;

// Template metafunction for checking if type `T` is a reference type. Default
// inhertis from `false_type`.
template <typename T>
struct is_reference : false_type {};

// Template metafunction specialization for lvalue reference types. Inherits
// from `true_type`.
template <typename T>
struct is_reference<T&> : true_type {};

// Template metafunction specialization for rvalue reference types. Inherits
// from `true_type`.
template <typename T>
struct is_reference<T&&> : true_type {};

// Transformation alias template that calls the `is_reference` metafunction and
// returns the resulting value.
template <typename T>
static constexpr bool is_reference_v = is_reference<T>::value;

// Template metafunction for checking if type `T` has a top-level const
// qualifier. Default inhertis from `false_type`.
//
// No reference will ever be const as only the value referenced can be const and
// the reference itself is not const-qualified. For pointer types, the pointer
// itself must be const, not the value pointed to.
template <typename T>
struct is_const : false_type {};

// Template metafunction specialization for top-level const-qualified types.
// Inherits from `true_type`.
template <typename T>
struct is_const<const T> : true_type {};

// Transformation alias template that calls the `is_const` metafunction and
// returns the resulting value.
template <typename T>
static constexpr bool is_const_v = is_const<T>::value;

// Template metafunction for checking if a type is a base C++ function type
// (e.g. int(const std::string&, float)).
//
// C++ base function types cannot be const-qualified or be references. To
// determine if a type is a C++ base function type, ensure a `const T` is still
// not const and that the type itself is not a reference.
template <typename T>
struct is_function : bool_constant<!is_const_v<const T> && !is_reference_v<T>> {
};

// Transformation alias template that calls the `is_function` metafunction and
// returns the resulting value.
template <typename T>
static constexpr bool is_function_v = is_function<T>::value;

namespace detail {

// Helper template metafunction that evalutes edge cases of type inputs to
// `is_convertible`.
//
// No C++ base function types or array types are convertible
// to any other type. If both types are `void`, then types are convertbile. If
// only one type is `void`, then the types are not convertible.
template <typename From, typename To,
          bool = (is_void_v<From> && is_void_v<To>) || is_function_v<To> ||
                 is_array_v<To>>
struct is_convertible_helper : bool_constant<is_void_v<From> && is_void_v<To>> {
};

// Helper template metafunction that evalutes normal cases of type inputs to
// `is_convertible`.
//
// If the `From` type can be passed by value to a function expecting a parameter
// of `To` type, then type is convertible.
template <typename From, typename To>
struct is_convertible_helper<From, To, false> {
private:
  template <typename T>
  static void helper(T) noexcept;

  template <typename F, typename T>
  static auto test(int) -> decltype(helper<T>(declval<F>()), true_type{});

  template <typename F, typename T>
  static auto test(...) -> false_type;

public:
  using type = decltype(test<From, To>(0));
};

}  // namespace detail

// Template metafunction to determine if an input type is convertbile to a
// target type. More specifically, if `From` is convertible to `To`. Inherits
// either `false_type` or `true_type`.
template <typename From, typename To>
struct is_convertible : detail::is_convertible_helper<From, To>::type {};

// Transformation alias template that calls the `is_convertible` metafunction
// and returns the resulting value.
template <typename From, typename To>
inline constexpr bool is_convertible_v = is_convertible<From, To>::value;

}  // namespace metafunctions
