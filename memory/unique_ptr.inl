namespace {

// Template metafunction fallback that aliases the `type` field to simply `T*`.
template <typename T, typename Deleter, typename = void>
struct pointer_selector {
  using type = T*;
};

// Template metafunction implementation that checks if `Deleter::pointer` exists and aliases `type` to that
// pointer type.
template <typename T, typename Deleter>
struct pointer_selector<T, Deleter, std::void_t<typename tmp_details::remove_reference_t<Deleter>::pointer>> {
  using type = typename tmp_details::remove_reference_t<Deleter>::pointer;
};

// Convenience alias that calls the `pointer_selector` metafunction.
template <typename T, typename Deleter>
using pointer_selector_t = typename pointer_selector<T, Deleter>::type;

} // namespace

// A functor that simply calls `delete` on the pointer passed in as an argument.
template <typename T>
struct default_deleter {
  using element_type = T;
  using pointer = element_type*;

  constexpr void operator()(T* data) const noexcept { delete data; }
};

