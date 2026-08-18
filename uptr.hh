// This file contains declarations for a custom unique pointer implementation.
// This class simply ties the lifetime of a heap-allocated pointer to the
// lifetime of an object of this class. A specialization for array types is
// included.
#pragma once

#include <cstddef>

#include "identity_trait_mfns.hh"
#include "type_modifying_mfns.hh"
#include "type_trait_mfns.hh"
#include "util.hh"

namespace mem {

// A functor that simply calls `delete` on the pointer passed in as an
// argument.
template <typename T>
struct default_deleter {
  using element_type = T;
  using pointer = element_type*;

  constexpr void operator()(T* data) const noexcept { delete data; }
};

// A default_deleter specialization for pointers to array types.
template <typename T>
struct default_deleter<T[]> {
  constexpr void operator()(T* data) const noexcept { delete[] data; }
};

// An RAII primitive type that binds the lifetime of a heap-allocated
// object to the lifetime of an object of this type. When a
// `unique_ptr` object goes out of scope, the heap-allocated memory is
// freed using the corresponding deleter.
template <typename T, typename Deleter = default_deleter<T>>
class unique_ptr {
private:
  // Template metafunction fallback case that assigns the struct's `type`
  // type to `Elem*`.
  template <typename Elem, typename Del, typename = void>
  struct try_deleter_pointer {
    using type = Elem*;
  };

  // Template metafunction specialization that attempts to set the struct's
  // `type ` to `Del::pointer`
  template <typename Elem, typename Del>
  struct try_deleter_pointer<
      Elem, Del,
      metafunctions::void_t<
          typename metafunctions::remove_reference_t<Del>::pointer>> {
    using type = metafunctions::remove_reference_t<Del>::pointer;
  };

  // Transformation alias template that calls the `try_deleter_pointer`
  // metafunctions and returns the resulting type.
  template <typename Elem, typename Del>
  using deleter_pointer_t = try_deleter_pointer<Elem, Del>::type;

public:
  using element_type = T;
  using pointer = deleter_pointer_t<element_type, Deleter>;
  using deleter_type = Deleter;

  // Default constructor for unique_ptr.
  [[nodiscard]] constexpr unique_ptr() noexcept;

  // Constructor for pointers convertible to `std::nullptr_t`.
  [[nodiscard]] constexpr unique_ptr(std::nullptr_t) noexcept;

  // Constructor that takes in heap-allocated pointer. Any non-heap-allocated
  // pointer will result in UB.
  [[nodiscard]] explicit constexpr unique_ptr(pointer data) noexcept;

  // Move constructor of identical type.
  [[nodiscard]] constexpr unique_ptr(unique_ptr&& other) noexcept;

  // Move assignment operator of identical type.
  [[nodiscard]] constexpr unique_ptr& operator=(unique_ptr&& other) noexcept;

  // Move constructor that enables type conversion. The moved-from type will
  // hold nullptr.
  template <typename U, typename OtherDeleter>
    requires(!metafunctions::is_array_v<U> &&
             metafunctions::is_convertible_v<
                 typename unique_ptr<U, OtherDeleter>::pointer, pointer> &&
             ((metafunctions::is_reference_v<OtherDeleter> &&
               metafunctions::is_same_v<Deleter, OtherDeleter>) ||
              (!metafunctions::is_reference_v<OtherDeleter> &&
               metafunctions::is_convertible_v<OtherDeleter, Deleter>)))
  [[nodiscard]] constexpr unique_ptr(
      unique_ptr<U, OtherDeleter>&& other) noexcept;

  // Move assignment operator that enables type conversion. The moved-from
  // type will hold nullptr.
  template <typename U, typename OtherDeleter>
    requires(!metafunctions::is_array_v<U> &&
             metafunctions::is_convertible_v<
                 typename unique_ptr<U, OtherDeleter>::pointer, pointer> &&
             ((metafunctions::is_reference_v<OtherDeleter> &&
               metafunctions::is_same_v<Deleter, OtherDeleter>) ||
              (!metafunctions::is_reference_v<OtherDeleter> &&
               metafunctions::is_convertible_v<OtherDeleter, Deleter>)))
  constexpr unique_ptr& operator=(unique_ptr<U, OtherDeleter>&& other) noexcept;

  // Destructor deletes the heap-allocated memory pointed to by `data_` if
  // `data_` is not `nullptr`.
  constexpr ~unique_ptr() noexcept;

  // Not copy-constructible.
  constexpr unique_ptr(const unique_ptr& other) = delete;

  // Not copy-assignable.
  constexpr void operator=(const unique_ptr& other) = delete;

  // Returns the pointer owned to the caller and sets the internal data field
  // to `nullptr`.
  [[nodiscard]] constexpr pointer release() noexcept;

  // Deletes the currently owned pointer and begins to own the pointer passed.
  constexpr void reset(pointer ptr = pointer()) noexcept;

  // Swaps the managed pointers between the two unique pointers.
  constexpr void swap(unique_ptr& other) noexcept;

  // Returns a pointer to the managed memory.
  [[nodiscard]] constexpr pointer get() const noexcept;

  // Returns a mutable reference to the pointers deleter.
  [[nodiscard]] constexpr deleter_type& get_deleter() noexcept;

  // Returns an immutable reference to the pointers deleter.
  [[nodiscard]] constexpr const deleter_type& get_deleter() const noexcept;

  // Dereferences the pointer to the heap-allocated object. Dereferencing
  // nullptr is UB. This method is only noexcept if the underlying type being
  // dereferenced is noexcept dereferencable.
  [[nodiscard]] constexpr
      typename metafunctions::add_lvalue_reference_t<element_type>
      operator*() const noexcept(noexcept(*util::declval<pointer>()));

  // Returns the underlying pointer owned by the unique pointer object. This
  // functions the same as `.get()`.
  [[nodiscard]] constexpr pointer operator->() const noexcept;

  // Returns true if the object owns a pointer and false if it does not own
  // anything.
  [[nodiscard]] constexpr explicit operator bool() const noexcept;

  // Returns true if the underlying pointers are to the same memory.
  [[nodiscard]] constexpr bool operator==(
      const unique_ptr& other) const noexcept;

  // Returns true if the object manages no underlying pointer.
  [[nodiscard]] constexpr bool operator==(std::nullptr_t) const noexcept;

private:
  // The pointer to the allocated memory to manage.
  pointer data_ = nullptr;

  // The deleter functor to delete the allocated memory with. The
  // no_unique_address attribute allows for a stateless deleter to not have a
  // memory footprint un unique_ptr objects.
  [[no_unique_address]] Deleter del_;
};

}  // namespace mem
