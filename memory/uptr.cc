#include <type_traits>

namespace {

template <typename T>
struct default_deleter {
  constexpr void operator()(T *data) const noexcept { delete data; };
};

template <typename T>
struct default_deleter<T[]> {
  constexpr void operator()(T *data) const noexcept { delete[] data; }
};

template <typename T, typename Deleter = default_deleter<T>>
class unique_ptr {
public:
  using element_type = T;
  using pointer = element_type *;

  constexpr unique_ptr() noexcept : unique_ptr(nullptr) {}

  constexpr explicit unique_ptr(T *data) noexcept : data_(data) {}

  template <typename U>
    requires std::is_convertible_v<U *, pointer>
  constexpr unique_ptr(unique_ptr<U> &&other) noexcept : data_(other.data_) {
    other.data_ = nullptr;
  }

  constexpr unique_ptr &operator=(unique_ptr &&other) noexcept {
    if (this != &other) {
      if (data_ != nullptr) {
        Deleter{}(data_);
      }
      data_ = other.data_;
      other.data_ = nullptr;
    }
    return *this;
  }

  constexpr ~unique_ptr() {
    if (data_ != nullptr) {
      Deleter{}(data_);
    }
  }

  void operator=(const unique_ptr &other) = delete;

  constexpr pointer release() noexcept {
    if (data_ != nullptr) {
      pointer data = data_;
      data_ = nullptr;
      return data;
    }
    return nullptr;
  }

  template <typename U>
    requires std::is_convertible_v<U *, pointer>
  constexpr void reset(U *new_data) noexcept {
    if (data_ != nullptr) {
      Deleter{}(data_);
    }
    data_ = new_data;
  }

  constexpr pointer get() const noexcept { return data_; }

  constexpr typename std::add_lvalue_reference<element_type>::type
  operator*() const noexcept {
    return *data_;
  }

  constexpr pointer operator->() const noexcept { return data_; }

  constexpr explicit operator bool() const noexcept { return data_ != nullptr; }

private:
  T *data_;
};

template <typename T, typename Deleter> class unique_ptr<T[], Deleter> {
public:
  using element_type = T;
  using pointer = element_type *;
  using delete_type = Deleter;
};

}; // namespace aux
