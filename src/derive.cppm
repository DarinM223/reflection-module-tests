export module derive;

import std;
import utils;

export template <auto V> struct Derive {};
export template <auto V> inline constexpr Derive<V> derive;
// Need symbol for anonymous struct for module internal linkage
export inline constexpr struct __Debug_Internal {
} Debug;
export struct format_as {
  std::meta::info type;
};

consteval std::meta::info format_type(std::meta::info type) {
  auto as = std::meta::annotations_of_with_type(type, ^^format_as);
  if (!as.empty()) {
    return std::meta::extract<format_as>(as[0]).type;
  }
  return type;
}

template <class T> struct derive_formatter {
  constexpr auto parse(auto &ctx) { return ctx.begin(); }
  auto format(T const &m, auto &ctx) const {
    auto out = std::format_to(ctx.out(), "{}", display_string_of(^^T));
    *out++ = '{';
    bool first = true;
    [:expand(nsdms(^^T)):] >> [&]<auto nsdm> {
      if (!first) {
        *out++ = ',';
        *out++ = ' ';
      }
      first = false;
      out = std::format_to(out, ".{} = {}", identifier_of(nsdm), m.[:nsdm:]);
    };
    *out++ = '}';
    return out;
  }
};

export template <class T>
  requires(has_annotation(^^T, derive<Debug>))
struct std::formatter<T> : derive_formatter<typename[:format_type(^^T):]> {};