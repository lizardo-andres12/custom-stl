// This file contains utility functions that can be used to improve QoL as a
// developer.
#pragma once

#include "type_modifying_mfns.hh"

namespace util {

// Allows the use of the type `T` in unevaluated contexts. This can be used to
// introspect return types or value types of an object that cannot be
// constructed.
template <typename T>
auto declval() -> metafunctions::add_rvalue_reference_t<T>;

}  // namespace util
