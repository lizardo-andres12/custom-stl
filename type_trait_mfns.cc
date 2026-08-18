// Static-assert test suite for the metafunctions declared in
//
// Covers happy-path behavior plus edge cases: void / cv-qualified void,
// cv-qualification generally, reference types, ordinary and "abominable"
// (cv/ref-qualified member) function types, bounded/unbounded arrays,
// lambdas, and inheritance hierarchies (public/private/protected, multiple,
// virtual, slicing).
//
// This file is expected to compile with zero diagnostics. If a
// static_assert fails, the compiler error identifies exactly which case
// broke and why (see the message string).

#include "type_trait_mfns.hh"

using namespace metafunctions;

namespace test_types {

// Base class for inheritance hierarchy tests.
struct Base {
  virtual ~Base() = default;
};

// Derived classes for inheritance hierarchy tests.
struct PublicDerived : public Base {};
struct PrivateDerived : private Base {};
struct ProtectedDerived : protected Base {};

// Default class unrelated to inheritance hierarchy.
struct Unrelated {};

// Base class for virtual inheritance hierarchy tests.
struct VBase {
  int x;
};

// Derived classes for virtual inheritance hierarchy tests.
struct VLeft : virtual VBase {};
struct VRight : virtual VBase {};
struct VDiamond : VLeft, VRight {};

// Base classes for multiple inheritance hierarchy tests.
struct MultiBase1 {
  int a;
};
struct MultiBase2 {
  int b;
};

// Mutiple inheritance dervied type.
struct MultiDerived : MultiBase1, MultiBase2 {};

// Explicitly constructible type.
struct ExplicitCtor {
  explicit ExplicitCtor(int);
};

// Loosely constructible type.
struct ImplicitCtor {
  ImplicitCtor(int);
};

// Lambda and functor types.
inline auto capless_lambda = [](int x) { return x + 1; };
inline auto capturing_lambda = [y = 1](int x) { return x + y; };
using CaplessLambdaT = decltype(capless_lambda);
using CapturingLambdaT = decltype(capturing_lambda);

struct Functor {
  int operator()(int x) const { return x; }
};

// Enumeration types.
enum class ScopedColor { Red, Green, Blue };
enum RawColor { RC_RED, RC_GREEN };

}  // namespace test_types

using namespace test_types;

namespace test_is_void {

static_assert(is_void_v<void>, "plain void must be void");
static_assert(is_void_v<const void>, "const void must be void");
static_assert(is_void_v<volatile void>, "volatile void must be void");
static_assert(is_void_v<const volatile void>, "cv void must be void");

static_assert(!is_void_v<int>, "int is not void");
static_assert(!is_void_v<void*>, "void* is a pointer, not void");
static_assert(!is_void_v<void()>, "a function type is not void");
static_assert(!is_void_v<decltype(nullptr)>, "nullptr_t is not void");
static_assert(!is_void_v<Base>, "a class type is not void");

}  // namespace test_is_void

namespace test_is_same {

// Readability alias that returns true if types are not the same and false if
// types are the same.
template <typename T, typename U>
constexpr bool is_not_same_v = !is_same_v<T, U>;

// happy path
static_assert(is_same_v<int, int>, "identical types are the same");
static_assert(is_same_v<void, void>, "void is the same as void");
static_assert(is_same_v<Base, Base>, "class type is the same as itself");

// cv-qualification matters
static_assert(is_not_same_v<int, const int>,
              "cv-qualification makes types different");
static_assert(is_not_same_v<int, volatile int>,
              "cv-qualification makes types different");
static_assert(is_same_v<const int, const int>,
              "matching cv-qualification is same");

// reference-ness matters
static_assert(is_not_same_v<int, int&>, "value vs lvalue-ref differ");
static_assert(is_not_same_v<int, int&&>, "value vs rvalue-ref differ");
static_assert(is_not_same_v<int&, int&&>, "lvalue-ref vs rvalue-ref differ");

// arrays: bound matters, decay does NOT happen
static_assert(is_same_v<int[3], int[3]>, "same bounded array types are same");
static_assert(is_not_same_v<int[3], int[4]>,
              "different bounds are different types");
static_assert(is_not_same_v<int[3], int*>,
              "is_same never decays arrays to pointers");
static_assert(is_not_same_v<int[], int[3]>,
              "unbounded vs bounded array differ");

// function types
static_assert(is_same_v<int(int), int(int)>,
              "identical function types are same");
static_assert(is_not_same_v<int(int), int(long)>,
              "different parameter types differ");
static_assert(is_not_same_v<int(int), void(int)>,
              "different return types differ");

// lambdas: every closure type is unique, even with identical signatures
static_assert(is_not_same_v<CaplessLambdaT, CapturingLambdaT>,
              "distinct lambda expressions have distinct closure types");
static_assert(is_same_v<CaplessLambdaT, CaplessLambdaT>,
              "a closure type is the same as itself");

// inheritance: is_same is not is_base_of — unrelated even if related by
// inheritance
static_assert(is_not_same_v<Base, PublicDerived>,
              "base and derived are different types");

}  // namespace test_is_same

namespace test_is_array {

template <typename T>
constexpr bool is_not_array_v = !is_array_v<T>;

// Unbounded arrays: correctly identified as array.
static_assert(is_array_v<int[]>, "unbounded array is an array");
static_assert(is_array_v<Base[]>, "unbounded array of class type is an array");

// bounded arrays: correctly identified as array.
static_assert(is_array_v<int[3]>, "bounded array is an array");
static_assert(is_array_v<int[3][4]>, "bounded 2D array is an array");

// non-array types: correctly identified as not array.
static_assert(is_not_array_v<int>, "int is not an array");
static_assert(is_not_array_v<int*>, "pointer is not an array");
static_assert(is_not_array_v<int&>, "reference is not an array");
static_assert(is_not_array_v<int (*)[3]>,
              "pointer to array is not itself an array");

}  // namespace test_is_array

namespace test_is_reference {

static_assert(is_reference_v<int&>, "lvalue reference is a reference");
static_assert(is_reference_v<int&&>, "rvalue reference is a reference");
static_assert(is_reference_v<const int&>,
              "const lvalue reference is a reference");
static_assert(is_reference_v<Base&>, "reference to class type is a reference");
static_assert(is_reference_v<int (&)[3]>, "reference to array is a reference");
static_assert(is_reference_v<int (&)(int)>,
              "reference to function is a reference");

static_assert(!is_reference_v<int>, "value type is not a reference");
static_assert(!is_reference_v<int*>,
              "pointer is not a reference (common confusion)");
static_assert(!is_reference_v<const int>,
              "cv-qualified value is not a reference");
static_assert(!is_reference_v<int[3]>,
              "array (non-reference) is not a reference");

}  // namespace test_is_reference

namespace test_is_const {

static_assert(is_const_v<const int>, "top-level const is detected");
static_assert(is_const_v<const Base>, "const class type is detected");
static_assert(is_const_v<int* const>,
              "const pointer (not pointee) is top-level const");
static_assert(is_const_v<const volatile int>,
              "const+volatile still counts as const");

static_assert(!is_const_v<int>, "plain int is not const");
static_assert(!is_const_v<const int*>,
              "pointer TO const is not itself top-level const");
static_assert(!is_const_v<volatile int>, "volatile alone is not const");

// references are never top-level const: "int& const" cannot be spelled, and
// "const" on the referred-to type doesn't make the reference itself const.
static_assert(!is_const_v<int&>, "a reference itself is never const");
static_assert(!is_const_v<const int&>,
              "const applies to the referent, not the reference");
static_assert(!is_const_v<const int&&>,
              "const applies to the referent, not the reference");

}  // namespace test_is_const

namespace test_is_function {

// happy path: ordinary function types
static_assert(is_function_v<int(int)>, "plain function type is a function");
static_assert(is_function_v<void()>, "no-arg function type is a function");
static_assert(is_function_v<int(int, ...)>,
              "variadic function type is a function");
static_assert(is_function_v<int(int) noexcept>,
              "noexcept function type is a function");

// "abominable" cv/ref-qualified member-function types — these are real,
// nameable types (via alias) even though they can't be free-function
// declarators. Real std::is_function_v is true for all of these.
static_assert(is_function_v<int(int) const>,
              "cv-qualified (abominable) function type");
static_assert(is_function_v<int(int) volatile>,
              "volatile-qualified abominable function type");
static_assert(is_function_v<int(int) const volatile>,
              "cv-qualified abominable function type");
static_assert(is_function_v<int(int) &>,
              "lvalue-ref-qualified abominable function type");
static_assert(is_function_v<int(int) &&>,
              "rvalue-ref-qualified abominable function type");
static_assert(is_function_v<int(int) const noexcept>,
              "cv-qualified + noexcept abominable function type");

// things that are commonly confused with function types but are not
static_assert(!is_function_v<int (*)(int)>,
              "function pointer is a pointer, not a function type");
static_assert(!is_function_v<int (&)(int)>,
              "function reference is a reference, not a function type");
static_assert(!is_function_v<int (Base::*)(int)>,
              "pointer-to-member-function is a pointer type");
static_assert(!is_function_v<CaplessLambdaT>,
              "a lambda's closure type is a class, not a function type");
static_assert(!is_function_v<Functor>,
              "a functor's class type is not a function type");
static_assert(!is_function_v<int>, "int is not a function type");
static_assert(!is_function_v<void>, "void is not a function type");

}  // namespace test_is_function

namespace test_is_convertible {

// happy path: arithmetic / qualification conversions
static_assert(is_convertible_v<int, double>,
              "int -> double is a standard conversion");
static_assert(is_convertible_v<int, long>,
              "int -> long is a standard conversion");
static_assert(is_convertible_v<char, int>,
              "char -> int is a standard (promotion) conversion");
static_assert(is_convertible_v<int, const int&>,
              "binding a const ref to a temporary is fine");
static_assert(!is_convertible_v<int*, double*>,
              "unrelated pointer types are not convertible");
static_assert(is_convertible_v<int*, const int*>,
              "adding cv-qualification via pointer is fine");
static_assert(!is_convertible_v<const int*, int*>,
              "removing cv-qualification implicitly is not fine");

// void handling
static_assert(is_convertible_v<void, void>,
              "void -> void is convertible (special-cased true)");
static_assert(!is_convertible_v<int, void>, "T -> void is never convertible");
static_assert(!is_convertible_v<void, int>, "void -> T is never convertible");

// function and array types as the target are never convertible-to
static_assert(!is_convertible_v<int, int(int)>,
              "nothing converts TO a function type");
static_assert(!is_convertible_v<int, int[3]>,
              "nothing converts TO an array type");

// array-to-pointer / function-to-pointer decay in the source position
static_assert(is_convertible_v<int[3], int*>,
              "array decays to pointer as an argument");
static_assert(is_convertible_v<int(int), int (*)(int)>,
              "function decays to function pointer");
static_assert(!is_convertible_v<int[3][4], int**>,
              "2D array decays to pointer-to-array, not pointer-to-pointer");
static_assert(is_convertible_v<int[3][4], int (*)[4]>,
              "2D array decays to the correct pointer type");

// explicit vs implicit constructors
static_assert(!is_convertible_v<int, ExplicitCtor>,
              "explicit constructors block implicit conversion");
static_assert(is_convertible_v<int, ImplicitCtor>,
              "non-explicit constructors allow conversion");

// inheritance hierarchies
static_assert(is_convertible_v<PublicDerived*, Base*>,
              "public-derived* -> base* is a standard conversion");
static_assert(!is_convertible_v<Base*, PublicDerived*>,
              "base* -> derived* needs an explicit cast");
static_assert(!is_convertible_v<PrivateDerived*, Base*>,
              "private inheritance blocks the conversion");
static_assert(!is_convertible_v<ProtectedDerived*, Base*>,
              "protected inheritance blocks the conversion");
static_assert(!is_convertible_v<Unrelated*, Base*>,
              "unrelated types are never convertible");
static_assert(is_convertible_v<PublicDerived, Base>,
              "slicing conversion (by value) is allowed");
static_assert(!is_convertible_v<Base, PublicDerived>,
              "no implicit narrowing/downcast by value");

// virtual and multiple inheritance
static_assert(
    is_convertible_v<VDiamond*, VBase*>,
    "diamond-shaped virtual inheritance still converts to the shared base");
static_assert(is_convertible_v<MultiDerived*, MultiBase1*>,
              "convertible to first base in multiple inheritance");
static_assert(is_convertible_v<MultiDerived*, MultiBase2*>,
              "convertible to second base in multiple inheritance");

// lambdas: capture-less lambdas decay to function pointers, capturing ones
// don't
static_assert(
    is_convertible_v<CaplessLambdaT, int (*)(int)>,
    "capture-less lambda is convertible to a matching function pointer");
static_assert(!is_convertible_v<CapturingLambdaT, int (*)(int)>,
              "a capturing lambda is NOT convertible to a function pointer");

// scoped vs unscoped enums
static_assert(is_convertible_v<RawColor, int>,
              "unscoped enum implicitly converts to int");
static_assert(!is_convertible_v<ScopedColor, int>,
              "scoped enum (enum class) does not implicitly convert to int");

}  // namespace test_is_convertible

int main() {}
