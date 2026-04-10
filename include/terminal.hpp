#pragma once
#include <iostream>
#include <string>

inline void terminal_enter_error()
{
  std::cerr << "\033[1;31m";
}

inline void terminal_exit_error()
{
  std::cerr << "\033[0m";
}

inline void terminal_write_error(const std::string& msg)
{
  terminal_enter_error();
  std::cerr << msg << '\n';
  terminal_exit_error();
}