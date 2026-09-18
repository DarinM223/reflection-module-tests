export module nanopass;

import std;

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

#define RECVISIT(expr) std::visit(std::forward<decltype(self)>(self), (expr))

template <typename Expr> struct LsrcVisitor {
  void operator()(VarExpr &) {}
  void operator()(IntExpr &) {}
  void operator()(this auto &&self, IfExpr1<Expr> &expr) {
    RECVISIT(*expr.cond);
    RECVISIT(*expr.thn);
  }
  void operator()(this auto &&self, IfExpr<Expr> &expr) {
    RECVISIT(*expr.cond);
    RECVISIT(*expr.thn);
    RECVISIT(*expr.els);
  }
  void operator()(this auto &&self, LamExpr<Expr> &expr) {
    RECVISIT(*expr.body);
  }
  void operator()(this auto &&self, AppExpr<Expr> &expr) {
    RECVISIT(*expr.f);
    RECVISIT(*expr.v);
  }
  void operator()(this auto &&self, LetExpr<Expr> &expr) {
    RECVISIT(*expr.expr);
    RECVISIT(*expr.body);
  }
};

template <typename Expr> struct L1Visitor : public LsrcVisitor<Expr> {
  using LsrcVisitor<Expr>::operator();
  void operator()(this auto &&self, IfExpr1<Expr> &expr) = delete;
  void operator()(IntExpr &) = delete;
  void operator()(VoidPlusIntExpr &) {}
};

void testCompile() {
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
}

using MyVariant = std::variant<int, float, double>;
using ModifiedVariant = [:modifyVariant(^^MyVariant, ^^Remove<float>,
                                        ^^Add<char, int>):];
static_assert(std::is_same_v<ModifiedVariant, std::variant<int, double, char>>);