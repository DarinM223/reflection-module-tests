export module utils;

import std;

export consteval auto nsdms(std::meta::info type)
    -> std::vector<std::meta::info> {
  return nonstatic_data_members_of(type, std::meta::access_context::current());
}

namespace __impl {
template <auto... vals> struct replicator_type {
  template <class F> constexpr void operator>>(F body) const {
    (body.template operator()<vals>(), ...);
  }
};

template <auto... vals> replicator_type<vals...> replicator = {};
} // namespace __impl

export template <class R> consteval auto expand(R range) {
  std::vector<std::meta::info> args;
  for (auto r : range) {
    args.push_back(std::meta::reflect_constant(r));
  }
  return std::meta::substitute(^^__impl::replicator, args);
}

export template <class T>
consteval bool has_annotation(std::meta::info type, T const &value) {
  // reflect_value changed to reflect_constant & value_of changed to constant_of
  auto expected_info = std::meta::reflect_constant(value);
  for (const std::meta::info info : std::meta::annotations_of(type)) {
    if (std::meta::constant_of(info) == expected_info) {
      return true;
    }
  }
  return false;
}