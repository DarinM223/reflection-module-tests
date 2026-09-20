export module nanopass;

import std;
import utils;

export template <class... Args> struct Remove;
export template <class... Args> struct Add;

export consteval auto modifyVariant(std::meta::info variant,
                                    std::meta::info remove,
                                    std::meta::info add) {
  // Extract the template argument reflections from each type.
  std::vector<std::meta::info> currentArgs =
      std::meta::template_arguments_of(std::meta::dealias(variant));
  std::vector<std::meta::info> toRemove =
      std::meta::template_arguments_of(std::meta::dealias(remove));
  std::vector<std::meta::info> toAdd =
      std::meta::template_arguments_of(std::meta::dealias(add));
  if (remove != std::meta::substitute(^^Remove, toRemove)) {
    throw std::meta::exception(u8"Remove type required for second parameter",
                               remove);
  }
  if (add != std::meta::substitute(^^Add, toAdd)) {
    throw std::meta::exception(u8"Add type required for third parameter", add);
  }

  std::vector<std::meta::info> finalArgs;

  // Remove specified types.
  for (auto arg : currentArgs) {
    if (std::ranges::find(toRemove, arg) == toRemove.end()) {
      finalArgs.push_back(arg);
    }
  }

  // Add new types (ensuring no duplicates).
  for (auto arg : toAdd) {
    if (std::ranges::find(finalArgs, arg) == finalArgs.end()) {
      finalArgs.push_back(arg);
    }
  }

  // Synthesize and return the new std::variant type using reflection.
  return std::meta::substitute(^^std::variant, finalArgs);
}

consteval std::meta::info unwrapPointerType(std::meta::info info) {
  std::meta::info target = std::meta::dealias(info);
  if (std::meta::is_pointer_type(target)) {
    return std::meta::remove_pointer(target);
  }
  if (std::meta::has_template_arguments(target)) {
    if (std::meta::template_of(target) == ^^std::unique_ptr) {
      auto args = std::meta::template_arguments_of(target);
      return args[0];
    }
  }
  return target;
}

/* Tests for unwrapPointerType */
using IntPtr = int *;
using IntUniquePtr = std::unique_ptr<int>;
using Int1 = [:unwrapPointerType(^^IntPtr):];
using Int2 = [:unwrapPointerType(^^IntUniquePtr):];
static_assert(std::is_same_v<Int1, int>);
static_assert(std::is_same_v<Int2, int>);

template <typename Expr, std::meta::info Type, typename Ret>
struct VisitorForType {
  static constexpr auto exprTy = ^^Expr;
  Ret operator()(this auto &&self, [:Type:] & expr) {
    std::println("Visiting the type {}", std::meta::display_string_of(Type));
    template for (constexpr auto info : members) {
      constexpr auto ty = std::meta::type_of(info);
      if constexpr (unwrapPointerType(ty) == exprTy) {
        std::visit(std::forward<decltype(self)>(self), *expr.[:info:]);
      }
    }
  }

private:
  static constexpr auto members = std::define_static_array(nsdms(Type));
};

template <typename L, typename Ret, typename... Ts>
struct MakeVisitor : public VisitorForType<L, ^^Ts, Ret>... {
  using VisitorForType<L, ^^Ts, Ret>::operator()...;
};

export template <typename L, typename Ret>
consteval std::meta::info makeVisitorFromVariant() {
  auto bases = std::meta::bases_of(^^L, std::meta::access_context::current());
  std::meta::info baseType = std::meta::type_of(bases[0]);
  std::meta::info underlyingVariant = std::meta::dealias(baseType);
  auto args = std::meta::template_arguments_of(underlyingVariant);
  std::vector<std::meta::info> fullArgs;
  fullArgs.push_back(^^L);
  fullArgs.push_back(^^Ret);
  for (auto arg : args) {
    fullArgs.push_back(arg);
  }
  return std::meta::substitute(^^MakeVisitor, fullArgs);
}

export template <typename Expr, typename L, typename Ret>
consteval std::meta::info makeVisitorFromVariantTemplate() {
  auto args = std::meta::template_arguments_of(^^L);
  std::vector<std::meta::info> fullArgs;
  fullArgs.push_back(^^Expr);
  fullArgs.push_back(^^Ret);
  for (auto arg : args) {
    fullArgs.push_back(arg);
  }
  return std::meta::substitute(^^MakeVisitor, fullArgs);
}

struct VarExpr {
  std::string var;
};

struct IntExpr {
  std::int32_t value;
};

template <typename Expr> struct IfExpr1 {
  std::unique_ptr<Expr> cond;
  std::unique_ptr<Expr> thn;
};

template <typename Expr> struct IfExpr {
  std::unique_ptr<Expr> cond;
  std::unique_ptr<Expr> thn;
  std::unique_ptr<Expr> els;
};

template <typename Expr> struct LamExpr {
  std::string var;
  std::unique_ptr<Expr> body;
};

template <typename Expr> struct AppExpr {
  std::unique_ptr<Expr> f;
  std::unique_ptr<Expr> v;
};

template <typename Expr> struct LetExpr {
  std::string id;
  std::unique_ptr<Expr> expr;
  std::unique_ptr<Expr> body;
};

struct VoidPlusIntExpr {
  std::int32_t value;
};

template <typename Expr>
using Lsrc_ = std::variant<VarExpr, IntExpr, IfExpr1<Expr>, IfExpr<Expr>,
                           LamExpr<Expr>, AppExpr<Expr>, LetExpr<Expr>>;
struct Lsrc : public Lsrc_<Lsrc> {
  using Lsrc_<Lsrc>::variant;
};

template <typename Expr>
using L1_ = [:modifyVariant(^^Lsrc_<Expr>, ^^Remove<IfExpr1<Expr>, IntExpr>,
                            ^^Add<VoidPlusIntExpr>):];
struct L1 : public L1_<L1> {
  using L1_<L1>::variant;
};

template <typename Expr>
using LsrcVisitor =
    typename[:makeVisitorFromVariantTemplate<Expr, Lsrc_<Expr>, void>():];

template <typename Expr> struct L1Visitor : public LsrcVisitor<Expr> {
  using LsrcVisitor<Expr>::operator();
  void operator()(this auto &&self, IfExpr1<Expr> &expr) = delete;
  void operator()(IntExpr &) = delete;
  void operator()(VoidPlusIntExpr &) {
    std::println("Visited VoidPlusIntExpr!");
  }
};

using LsrcVisitor2 = typename[:makeVisitorFromVariant<Lsrc, void>():];
using L1Visitor2 = typename[:makeVisitorFromVariant<L1, void>():];
template <typename Expr>
using L1Visitor3 =
    typename[:makeVisitorFromVariantTemplate<Expr, L1_<Expr>, void>():];

export void testCompile() {
  Lsrc expr1 = IfExpr{.cond = std::make_unique<Lsrc>(IntExpr{.value = 1}),
                      .thn = std::make_unique<Lsrc>(IntExpr{.value = 2}),
                      .els = std::make_unique<Lsrc>(IntExpr{.value = 3})};
  LsrcVisitor<Lsrc> visitor1;
  std::visit(visitor1, expr1);

  L1 expr2 = IfExpr{.cond = std::make_unique<L1>(VoidPlusIntExpr{.value = 1}),
                    .thn = std::make_unique<L1>(VoidPlusIntExpr{.value = 2}),
                    .els = std::make_unique<L1>(VoidPlusIntExpr{.value = 3})};
  L1Visitor<L1> visitor2;
  std::visit(visitor2, expr2);

  LsrcVisitor2 visitor3;
  std::visit(visitor3, expr1);

  L1Visitor2 visitor5;
  std::visit(visitor5, expr2);
  L1Visitor3<L1> visitor6;
  std::visit(visitor6, expr2);
}

using MyVariant = std::variant<int, float, double>;
using ModifiedVariant = [:modifyVariant(^^MyVariant, ^^Remove<float>,
                                        ^^Add<char, int>):];
static_assert(std::is_same_v<ModifiedVariant, std::variant<int, double, char>>);