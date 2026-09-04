export module derive;

import std;
import utils;

export template <auto V> struct Derive {};
export template <auto V> inline constexpr Derive<V> derive;
// Need symbol for anonymous struct for module internal linkage
export inline constexpr struct __Debug_Internal {
} Debug;

export template <class T>
  requires(has_annotation(^^T, derive<Debug>))
struct std::formatter<T> {
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