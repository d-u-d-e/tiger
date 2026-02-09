#include "ir/level.hpp"
#include "mock_frame.hpp"
#include "semant/entry.hpp"
#include "semant/env.hpp"
#include "semant/types.hpp"
#include "temp.hpp"
#include <doctest/doctest.h>
#include <symbol.hpp>
#include <vector>

using namespace semant;

template <typename ExpectedType, bool found = true>
auto lookup_tentry = [](Environment<TEntry>& tenv, const Symbol& s) {
  auto lookup = tenv.lookup(s);
  CHECK((lookup != nullptr) == found);
  if constexpr(found)
  {
    auto t = dynamic_cast<ExpectedType*>(lookup->t.get());
    CHECK(t != nullptr);
  }
};

template <typename FrameT, typename T, bool expected = true>
auto lookup_ventry = [](Environment<VEntry<FrameT>>& venv, const Symbol& s) {
  auto lookup = venv.lookup(s);
  CHECK((lookup != nullptr) == expected);
  if constexpr(expected)
  {
    CHECK(std::holds_alternative<T>(lookup->v));
  }
};

TEST_SUITE("environment")
{
  using namespace semant;

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
    auto T = Symbol("T", 1);
    auto x = Symbol("x", 2);
    auto y = Symbol("y", 3);
    auto z = Symbol("z", 4);
    auto R = Symbol("R", 5);
    auto A = Symbol("A", 6);

    tenv.begin_scope();
    tenv.enter(T, TEntry{std::make_shared<Integer>()});
    lookup_tentry<Integer>(tenv, T);

    tenv.begin_scope();
    tenv.enter(T, TEntry{std::make_shared<String>()});
    lookup_tentry<String>(tenv, T);

    tenv.end_scope();
    lookup_tentry<Integer>(tenv, T);

    std::vector<std::pair<Symbol, SharedType>> fields;
    fields.emplace_back(x, std::make_shared<Integer>());
    fields.emplace_back(y, std::make_shared<String>());
    fields.emplace_back(z, std::make_shared<String>());
    tenv.enter(R, TEntry{std::make_shared<Record>(fields)});

    tenv.enter(A, TEntry{std::make_shared<Array>(std::make_shared<types::String>())});

    lookup_tentry<Record>(tenv, R);
    lookup_tentry<Array>(tenv, A);

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
    using LevelImpl = Level<mock::Frame>;
    Environment<VEntry<mock::Frame>> venv;
    auto a = Symbol("a", 1);
    auto b = Symbol("b", 2);
    auto f = Symbol("f", 3);
    auto g = Symbol("g", 4);
    auto x = Symbol("x", 5);
    auto y = Symbol("y", 6);

    LevelImpl::Access ax; // dummy
    std::shared_ptr<LevelImpl> l = nullptr; // dummy

    venv.begin_scope();
    venv.enter(a, SimpleVarEntry<mock::Frame>(std::make_shared<Integer>(), ax));
    lookup_ventry<mock::Frame, SimpleVarEntry<mock::Frame>>(venv, a);

    venv.begin_scope();
    venv.enter(b, SimpleVarEntry<mock::Frame>(std::make_shared<types::String>(), ax));
    lookup_ventry<mock::Frame, SimpleVarEntry<mock::Frame>>(venv, b);

    std::vector<types::SharedType> formals;
    formals.push_back(std::make_shared<types::Integer>());
    formals.push_back(std::make_shared<types::String>());
    venv.enter(
      f,
      ClosureEntry(TempGen::new_label(),
                   std::make_shared<FunctionType>(formals, std::make_shared<types::String>()),
                   l,
                   ax));
    lookup_ventry<mock::Frame, ClosureEntry<mock::Frame>>(venv, f);

    venv.end_scope();
    lookup_ventry<mock::Frame, SimpleVarEntry<mock::Frame>, false>(venv, b);
    lookup_ventry<mock::Frame, SimpleVarEntry<mock::Frame>, false>(venv, f);

    formals.clear();
    formals.push_back(std::make_shared<types::String>());
    venv.enter(
      g,
      ClosureEntry(TempGen::new_label(),
                   std::make_shared<FunctionType>(formals, std::make_shared<types::String>()),
                   l,
                   ax));
    lookup_ventry<mock::Frame, ClosureEntry<mock::Frame>>(venv, g);

    venv.end_scope();
    CHECK(venv.size() == 0);
  }
}