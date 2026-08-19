// This file contains utility metafunctions that can be used to improve
// developer QoL.
#pragma once

#include "metafunctions/type_modifying_mfns.hh"

namespace metafunctions {

// Allows the use of the type `T` in unevaluated contexts. This can be used to
// introspect return types or value types of an object that cannot be
// constructed.
template <typename T>
auto declval() -> metafunctions::add_rvalue_reference_t<T>;

}  // namespace metafunctions
