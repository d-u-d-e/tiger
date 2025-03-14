#include <doctest/doctest.h>
#include <semantic/env.hpp>

TEST_SUITE_BEGIN("environment");
using namespace semantic::env;

template <typename T, bool expected = true>
auto lookup_tentry = [](Environment<TEntry>& tenv, const symbol::Symbol& s) {
  auto lookup = tenv.lookup(s);
  CHECK((lookup != nullptr) == expected);
  if constexpr(expected) {
    auto t = dynamic_cast<T*>(lookup->t.get());
    CHECK(t != nullptr);
  }
};

template <typename T, bool expected = true>
auto lookup_ventry = [](Environment<VEntry>& venv, const symbol::Symbol& s) {
  auto lookup = venv.lookup(s);
  CHECK((lookup != nullptr) == expected);
  if constexpr(expected) {
    CHECK(std::holds_alternative<T>(*lookup));
  }
};

TEST_CASE("nested_scopes_types.tig")
{
  // We are basically testing the environment for the following program

  /*
    let
      type T = int
      let 
        type T = string
       in
      end
      type R = {x: int, y: string, z: string}
      type A = array of string
    in
    end
  */

  Environment<TEntry> tenv;
  auto T = symbol::Symbol("T", 1);
  auto x = symbol::Symbol("x", 2);
  auto y = symbol::Symbol("y", 3);
  auto z = symbol::Symbol("z", 4);
  auto R = symbol::Symbol("R", 5);
  auto A = symbol::Symbol("A", 6);

  tenv.begin_scope();
  tenv.enter(T, TEntry{std::make_shared<semantic::types::Integer>()});
  lookup_tentry<semantic::types::Integer>(tenv, T);

  tenv.begin_scope();
  tenv.enter(T, TEntry{std::make_shared<semantic::types::String>()});
  lookup_tentry<semantic::types::String>(tenv, T);

  tenv.end_scope();
  lookup_tentry<semantic::types::Integer>(tenv, T);

  std::vector<std::pair<symbol::Symbol, std::shared_ptr<semantic::types::Type>>>
    fields;
  fields.emplace_back(x, std::make_shared<semantic::types::Integer>());
  fields.emplace_back(y, std::make_shared<semantic::types::String>());
  fields.emplace_back(z, std::make_shared<semantic::types::String>());
  tenv.enter(R, TEntry{std::make_shared<semantic::types::Record>(fields)});

  tenv.enter(A,
             TEntry{std::make_shared<semantic::types::Array>(
               std::make_shared<semantic::types::String>())});

  lookup_tentry<semantic::types::Record>(tenv, R);
  lookup_tentry<semantic::types::Array>(tenv, A);

  tenv.end_scope();
  CHECK(tenv.size() == 0);
}

TEST_CASE("nested_scopes_vars_funcs.tig")
{
  // We are basically testing the environment for the following program

  /*
    let
      var a := 2
      let 
        var b : string := "hello"
        function f(x: int, y: string): string = x
       in
      end
      function g(x: string): string = x
    in
    end
  */

  Environment<VEntry> venv;
  auto a = symbol::Symbol("a", 1);
  auto b = symbol::Symbol("b", 2);
  auto f = symbol::Symbol("f", 3);
  auto g = symbol::Symbol("g", 4);
  auto x = symbol::Symbol("x", 5);
  auto y = symbol::Symbol("y", 6);

  translation::Level::Access ax; // dummy
  std::shared_ptr<translation::Level> l; // dummy

  venv.begin_scope();
  venv.enter(a, VarEntry(std::make_shared<semantic::types::Integer>(), ax));
  lookup_ventry<VarEntry>(venv, a);

  venv.begin_scope();
  venv.enter(b, VarEntry(std::make_shared<semantic::types::String>(), ax));
  lookup_ventry<VarEntry>(venv, b);

  std::vector<std::shared_ptr<semantic::types::Type>> formals;
  formals.push_back(std::make_shared<semantic::types::Integer>());
  formals.push_back(std::make_shared<semantic::types::String>());
  venv.enter(f,
             FuncEntry(formals, std::make_shared<semantic::types::String>(), l));
  lookup_ventry<FuncEntry>(venv, f);

  venv.end_scope();
  lookup_ventry<VarEntry, false>(venv, b);
  lookup_ventry<VarEntry, false>(venv, f);

  formals.clear();
  formals.push_back(std::make_shared<semantic::types::String>());
  venv.enter(
    g, FuncEntry(formals, std::make_shared<semantic::types::String>(), l));
  lookup_ventry<FuncEntry>(venv, g);

  venv.end_scope();
  CHECK(venv.size() == 0);
}

TEST_SUITE_END();