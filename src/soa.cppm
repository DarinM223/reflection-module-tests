export module soa;

import std;
import utils;
import derive;

export template <class T> struct SoaVector {
  struct Pointers;
  struct RefBase;
  consteval {
    define_aggregate(
        ^^Pointers,
        nsdms(^^T) | std::views::transform([](std::meta::info member) {
          return data_member_spec(add_pointer(type_of(member)),
                                  {.name = identifier_of(member)});
        }));

    define_aggregate(
        ^^RefBase,
        nsdms(^^T) | std::views::transform([](std::meta::info member) {
          return data_member_spec(add_lvalue_reference(type_of(member)),
                                  {.name = identifier_of(member)});
        }));
  }

  struct[[ = derive<Debug>, = format_as{^^T} ]] Ref : RefBase {
    void operator=(T const &value) {
      template for (constexpr auto I : std::views::iota(0zu, mems.size())) {
        this->[:ref_mems[I]:] = value.[:mems[I]:];
      }
    }

    // Autoconversion necessary for format_as to work
    operator T() const {
      return [:expand(ref_mems):]
          << [this]<auto... M> { return T{this->[:M:]...}; };
    }
  };

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

  Ref operator[](std::size_t idx) {
    return [:expand(ptr_mems):]
        << [this, idx]<auto... M> { return Ref{pointers_.[:M:][idx]...}; };
  }
  T operator[](std::size_t idx) const {
    return [:expand(ptr_mems):]
        << [this, idx]<auto... M> { return T{pointers_.[:M:][idx]...}; };
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
  static constexpr auto ref_mems = std::define_static_array(nsdms(^^RefBase));

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
