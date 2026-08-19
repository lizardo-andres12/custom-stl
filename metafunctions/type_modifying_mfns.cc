// Static-assert test suite for the metafunctions declared in
// type_modifying_mfns.hh: remove_reference, add_lvalue_reference,
// add_rvalue_reference.
//
// Covers happy-path behavior plus edge cases: void (you can't form void&,
// so add_*_reference must fall back to leaving it as void), reference
// collapsing (adding an rvalue-ref to an lvalue-ref collapses to an
// lvalue-ref — a classic gotcha), ordinary function types (references to
// functions ARE formable), "abominable" cv/ref-qualified member function
// types (references to THOSE are not formable, so add_*_reference must
// fall back and leave them unchanged), arrays, cv-qualification, and
// idempotence / round-tripping between the three metafunctions.
//
// This file is expected to compile with zero diagnostics. Since these are
// all type-transforming (not bool-valued) metafunctions, each check is
// expressed as `static_assert(is_same_v<Actual, Expected>, ...)` using the
// is_same metafunction from type_trait_mfns.hh, which was already verified
// independently in type_trait_mfns_test.cc.

#include "metafunctions/type_modifying_mfns.hh"
#include "metafunctions/type_trait_mfns.hh"

using namespace metafunctions;

namespace test_types {

struct Base {
  virtual ~Base() = default;
};
struct Derived : Base {};

// An "abominable" function type: cv/ref-qualified function types that are
// nameable via alias but cannot have a reference formed to them (you can't
// write `int(int) const &`, nor a pointer/reference to one).
using ConstMemFn = int(int) const;
using RefQualMemFn = int(int) &;
using ConstNoexceptMemFn = int(int) const noexcept;

}  // namespace test_types

using namespace test_types;

namespace test_remove_reference {

// happy path
static_assert(is_same_v<remove_reference_t<int&>, int>, "strip lvalue ref");
static_assert(is_same_v<remove_reference_t<int&&>, int>, "strip rvalue ref");
static_assert(is_same_v<remove_reference_t<int>, int>,
              "non-reference is unchanged");

// cv-qualification is preserved — only the reference-ness is stripped
static_assert(is_same_v<remove_reference_t<const int&>, const int>,
              "const on the referent survives stripping the reference");
static_assert(is_same_v<remove_reference_t<volatile int&&>, volatile int>,
              "volatile on the referent survives stripping the reference");
static_assert(is_same_v<remove_reference_t<const int>, const int>,
              "cv-qualified non-reference is unchanged");

// pointers are not references — must be left alone
static_assert(is_same_v<remove_reference_t<int*>, int*>,
              "pointer is not a reference, unaffected");
static_assert(is_same_v<remove_reference_t<int*&>, int*>,
              "reference-to-pointer strips to the pointer");

// arrays and functions, by reference
static_assert(is_same_v<remove_reference_t<int (&)[3]>, int[3]>,
              "reference to array strips to the array");
static_assert(is_same_v<remove_reference_t<int (&&)[3]>, int[3]>,
              "rvalue reference to array strips to the array");
static_assert(is_same_v<remove_reference_t<int (&)(int)>, int(int)>,
              "reference to function strips to the function type");

// class types
static_assert(is_same_v<remove_reference_t<Base&>, Base>,
              "reference to class type strips correctly");
static_assert(is_same_v<remove_reference_t<Derived&&>, Derived>,
              "rvalue reference to class type strips correctly");

// idempotence: removing a reference from an already-stripped type is a no-op
static_assert(is_same_v<remove_reference_t<remove_reference_t<int&>>, int>,
              "remove_reference is idempotent");

}  // namespace test_remove_reference

namespace test_add_lvalue_reference {

// happy path
static_assert(is_same_v<add_lvalue_reference_t<int>, int&>,
              "plain type gets an lvalue ref");
static_assert(is_same_v<add_lvalue_reference_t<const int>, const int&>,
              "cv-qualification is preserved when adding the reference");
static_assert(is_same_v<add_lvalue_reference_t<Base>, Base&>,
              "class type gets an lvalue ref");

// reference collapsing: T& & -> T&, and T&& & -> T& (the classic rule)
static_assert(is_same_v<add_lvalue_reference_t<int&>, int&>,
              "adding lvalue-ref to an lvalue-ref collapses to lvalue-ref");
static_assert(
    is_same_v<add_lvalue_reference_t<int&&>, int&>,
    "adding lvalue-ref to an rvalue-ref collapses to lvalue-ref, NOT int&&&");

// void: you cannot form `void&`, so the SFINAE fallback must leave it as void
static_assert(
    is_same_v<add_lvalue_reference_t<void>, void>,
    "void& is ill-formed; SFINAE fallback leaves plain void unchanged");
static_assert(is_same_v<add_lvalue_reference_t<const void>, const void>,
              "same fallback for cv-qualified void");
static_assert(
    is_same_v<add_lvalue_reference_t<const volatile void>, const volatile void>,
    "same fallback for cv-qualified void");

// ordinary function types: references TO functions are perfectly formable
static_assert(
    is_same_v<add_lvalue_reference_t<int(int)>, int (&)(int)>,
    "an ordinary function type CAN have an lvalue reference formed to it");

// abominable (cv/ref-qualified member) function types: references to these
// cannot be formed at all, so the SFINAE fallback must leave them unchanged
static_assert(
    is_same_v<add_lvalue_reference_t<ConstMemFn>, ConstMemFn>,
    "you cannot form a reference to a cv-qualified abominable function "
    "type; fallback leaves it unchanged");
static_assert(is_same_v<add_lvalue_reference_t<RefQualMemFn>, RefQualMemFn>,
              "same fallback for a ref-qualified abominable function type");
static_assert(
    is_same_v<add_lvalue_reference_t<ConstNoexceptMemFn>, ConstNoexceptMemFn>,
    "same fallback for a cv+noexcept abominable function type");

// arrays: references to arrays ARE formable
static_assert(is_same_v<add_lvalue_reference_t<int[3]>, int (&)[3]>,
              "array type gets a reference-to-array, not decayed to pointer");
static_assert(is_same_v<add_lvalue_reference_t<int[]>, int (&)[]>,
              "unbounded array also gets a reference-to-array");

// pointers are ordinary objects here, nothing special
static_assert(is_same_v<add_lvalue_reference_t<int*>, int*&>,
              "pointer gets an lvalue ref like any object type");

}  // namespace test_add_lvalue_reference

namespace test_add_rvalue_reference {

// happy path
static_assert(is_same_v<add_rvalue_reference_t<int>, int&&>,
              "plain type gets an rvalue ref");
static_assert(is_same_v<add_rvalue_reference_t<const int>, const int&&>,
              "cv-qualification is preserved when adding the reference");
static_assert(is_same_v<add_rvalue_reference_t<Base>, Base&&>,
              "class type gets an rvalue ref");

// reference collapsing: T& && -> T& (lvalue-ref wins), T&& && -> T&&
static_assert(
    is_same_v<add_rvalue_reference_t<int&>, int&>,
    "adding rvalue-ref to an EXISTING lvalue-ref still collapses to lvalue-ref "
    "(this is why declval<T&> yields T&, not T&&)");
static_assert(is_same_v<add_rvalue_reference_t<int&&>, int&&>,
              "adding rvalue-ref to an rvalue-ref stays an rvalue-ref");

// void: same fallback story as add_lvalue_reference
static_assert(
    is_same_v<add_rvalue_reference_t<void>, void>,
    "void&& is ill-formed; SFINAE fallback leaves plain void unchanged");
static_assert(is_same_v<add_rvalue_reference_t<const void>, const void>,
              "same fallback for cv-qualified void");

// ordinary function types: rvalue references to functions are formable
// (unusual in practice, but well-formed)
static_assert(
    is_same_v<add_rvalue_reference_t<int(int)>, int (&&)(int)>,
    "an ordinary function type CAN have an rvalue reference formed to it");

// abominable function types: same fallback as the lvalue case
static_assert(
    is_same_v<add_rvalue_reference_t<ConstMemFn>, ConstMemFn>,
    "cannot form a reference to a cv-qualified abominable function type; "
    "fallback leaves it unchanged");
static_assert(is_same_v<add_rvalue_reference_t<RefQualMemFn>, RefQualMemFn>,
              "same fallback for a ref-qualified abominable function type");

// arrays
static_assert(is_same_v<add_rvalue_reference_t<int[3]>, int (&&)[3]>,
              "array type gets an rvalue-reference-to-array");

}  // namespace test_add_rvalue_reference

namespace test_round_trips {

// add then remove gets you back to the unqualified reference-free type
static_assert(is_same_v<remove_reference_t<add_lvalue_reference_t<int>>, int>,
              "add lvalue ref then remove it round-trips to the original type");
static_assert(is_same_v<remove_reference_t<add_rvalue_reference_t<int>>, int>,
              "add rvalue ref then remove it round-trips to the original type");

}  // namespace test_round_trips

int main() {}
