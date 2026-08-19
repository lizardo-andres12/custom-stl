// This file contains metafunctions used to manipulate types and and check
// invariants at compile time.
#pragma once

#include "metafunctions/identity_trait_mfns.hh"

namespace metafunctions {

namespace detail {

// Primary overload helper that attempts to form type `T&`.
template <typename T>
auto try_add_lvalue_ref(int) -> type_identity<T&>;

// Default overload helper that forms the type `T`. This is the fallback
// overload for `try_add_lvalue_ref`.
template <typename T>
auto try_add_lvalue_ref(...) -> type_identity<T>;

// Primary overload helper that attempts to form type `T&&`.
template <typename T>
auto try_add_rvalue_ref(int) -> type_identity<T&&>;

// Default overload helper that forms the type `T`. This is the fallback
// overload for `try_add_rvalue_ref`.
template <typename T>
auto try_add_rvalue_ref(...) -> type_identity<T>;

}  // namespace detail

// Template metafunction for getting the raw type of the template argument with
// no reference qualifiers.
template <typename T>
struct remove_reference {
  using type = T;
};

// Template metafunction specialization for types with lvalue reference
// qualifier.
template <typename T>
struct remove_reference<T&> {
  using type = T;
};

// Template metafunction specialization for types with rvalue reference
// qualifier.
template <typename T>
struct remove_reference<T&&> {
  using type = T;
};

// Transformation alias template that calls the `remove_reference` metafunction
// and returns the resulting type.
template <typename T>
using remove_reference_t = typename remove_reference<T>::type;

// Metafunction that uses SFINAE to try to add an lvalue reference to the input
// type.
template <typename T>
struct add_lvalue_reference
    : public decltype(detail::try_add_lvalue_ref<T>(0)) {};

// Transformation alias template that calls the `add_lvalue_reference`
// metafunction and retusn the resulting type.
template <typename T>
using add_lvalue_reference_t = typename add_lvalue_reference<T>::type;

// Metafunction that uses SFINAE to try to add an rvalue reference to the input
// type.
template <typename T>
struct add_rvalue_reference
    : public decltype(detail::try_add_rvalue_ref<T>(0)) {};

// Transformation alias template that calls the `add_rvalue_reference`
// metafunction and retusn the resulting type.
template <typename T>
using add_rvalue_reference_t = typename add_rvalue_reference<T>::type;

}  // namespace metafunctions
