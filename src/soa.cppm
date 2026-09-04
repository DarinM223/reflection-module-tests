export module soa;

import std;
import utils;

export template <class T> struct SoaVector {
  struct Pointers;
  consteval {
    define_aggregate(
        ^^Pointers,
        nsdms(^^T) | std::views::transform([](std::meta::info member) {
          return data_member_spec(add_pointer(type_of(member)),
                                  {.name = identifier_of(member)});
        }));
  }

  void push_back(T const &value) {
    if (size_ == capacity_) {
      grow(std::max(3 * size_ / 2, size_ + 2));
    }

    template for (constexpr auto I : std::views::iota(0zu, mems.size())) {
      constexpr auto from = mems[I];
      constexpr auto to = ptr_mems[I];

      using M = [:std::meta::type_of(from):];
      ::new (pointers_.[:to:] + size_) M(value.[:from:]);
    }

    ++size_;
  }

  ~SoaVector() {
    template for (constexpr auto M : ptr_mems) {
      delete_range(pointers_.[:M:]);
    }
  }

  Pointers pointers_ = {};
  std::size_t size_ = 0;
  std::size_t capacity_ = 0;

private:
  static constexpr auto mems = std::define_static_array(nsdms(^^T));
  static constexpr auto ptr_mems = std::define_static_array(nsdms(^^Pointers));

  void grow(std::size_t new_capacity) {
    Pointers new_pointers = {};
    template for (constexpr auto M : ptr_mems) {
      new_pointers.[:M:] = allocate<typename[:std::meta::remove_pointer(
                                                  std::meta::type_of(M)):]>(
                             new_capacity);
      std::uninitialized_copy_n(pointers_.[:M:], size_, new_pointers.[:M:]);
      delete_range(pointers_.[:M:]);
    }
    pointers_ = new_pointers;
    capacity_ = new_capacity;
  }

  template <class U> void delete_range(U *ptr) {
    std::destroy(ptr, ptr + size_);
    std::allocator<U>().deallocate(ptr, capacity_);
  }
  template <class U> U *allocate(std::size_t capacity) {
    return std::allocator<U>().allocate(capacity);
  }
};
