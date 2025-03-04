#include <doctest/doctest.h>
#include <semantic/env.hpp>

TEST_SUITE_BEGIN("environment");
using namespace semantic::environment;

template <typename T>
auto lookup_and_verify =
  [](Environment<TEntry>& tenv, const symbol::Symbol& s) {
    CHECK_FALSE(!tenv.lookup(s).has_value());
    auto t = dynamic_cast<T*>(tenv.lookup(s).value().get());
    CHECK_FALSE(t == nullptr);
  };

TEST_CASE("nested_scopes_types.tig")
{
  // We are basically testing the environment for the following program

  /*
    let
      type T := int
      let 
        type T := string
       in
      end
      type R := {x: int, y: string, z: string}
      type A := array of string
    in
    end
  */

  Environment<VEntry> venv;
  Environment<TEntry> tenv;

  auto T = symbol::Symbol("T", 1);
  auto x = symbol::Symbol("x", 2);
  auto y = symbol::Symbol("y", 3);
  auto z = symbol::Symbol("z", 4);
  auto R = symbol::Symbol("R", 5);
  auto A = symbol::Symbol("A", 6);

  tenv.begin_scope();
  tenv.enter(T, std::make_shared<semantic::types::Integer>());
  lookup_and_verify<semantic::types::Integer>(tenv, T);

  tenv.begin_scope();
  tenv.enter(T, std::make_shared<semantic::types::String>());
  lookup_and_verify<semantic::types::String>(tenv, T);

  tenv.end_scope();

  /*CHECK_FALSE(tenv.lookup(T).has_value());
  CHECK_FALSE(typeid(*tenv.lookup(T).value()) !=
              typeid(std::shared_ptr<semantic::types::Integer>));

  std::vector<std::pair<symbol::Symbol, std::shared_ptr<semantic::types::Type>>>
    fields;
  fields.emplace_back(x, std::make_shared<semantic::types::Integer>());
  fields.emplace_back(y, std::make_shared<semantic::types::String>());
  fields.emplace_back(z, std::make_shared<semantic::types::String>());
  tenv.enter(R, std::make_shared<semantic::types::Record>(fields, 0));

  tenv.enter(A,
             std::make_shared<semantic::types::Array>(
               std::make_shared<semantic::types::String>(), 1));*/
}

TEST_SUITE_END();