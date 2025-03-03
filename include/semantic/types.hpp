#pragma once
#include <memory>
#include <optional>
#include <symbol.hpp>
#include <vector>

namespace semantic
{

namespace types
{

class Type {
  public:
  virtual ~Type() = default;
  virtual std::string to_string() = 0;
};

struct Integer : public Type {
  std::string to_string() override
  {
    return "int";
  }
};
struct String : public Type {
  std::string to_string() override
  {
    return "string";
  }
};
struct Nil : public Type {
  std::string to_string() override
  {
    return "nil";
  }
};

// Used to indicate that an expression returns no value
struct Unit : public Type {
  std::string to_string() override
  {
    return "unit";
  }
};

struct Record : public Type {
  Record(std::vector<std::pair<symbol::Symbol, std::shared_ptr<Type>>> fields,
         uint32_t unique)
    : fields(std::move(fields))
    , unique(unique)
  { }

  bool operator==(const Record& other) const
  {
    return unique == other.unique;
  }

  std::string to_string() override
  {
    std::string result = "Record(" + std::to_string(unique) + "){";
    /*for(const auto& [name, type] : fields) {
      result +=
        "  " + name.name() + ": " +
        (typeid(type) == typeid(Record) ? "Record" : type->to_string()) + "\n";
    }*/
    auto size = fields.size();

    for (auto i = 0; i < size; i++) {
      auto& [name, type] = fields[i];
      result += name.name() + ": " + type->to_string() + (i == size - 1 ? "" : ", ");
    }
    return result + "}";
  }

  std::vector<std::pair<symbol::Symbol, std::shared_ptr<Type>>> fields;
  uint32_t unique;
};

struct Array : public Type {
  Array(std::shared_ptr<Type> type, uint32_t unique)
    : type(std::move(type))
    , unique(unique)
  { }

  bool operator==(const Array& other) const
  {
    return unique == other.unique;
  }

  std::string to_string() override
  {
    std::string result = "Array(" + std::to_string(unique) + ")";
    return result;
  }

  std::shared_ptr<Type> type;
  uint32_t unique;
};

struct Name : public Type {
  Name(const symbol::Symbol& name, std::shared_ptr<Type> type)
    : name(name)
    , type(std::move(type))
  { }

  symbol::Symbol name;
  std::shared_ptr<Type> type;
};

} // namespace types

}; // namespace semantic