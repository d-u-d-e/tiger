#pragma once
#include <iostream>

inline void terminal_enter_error()
{
  std::cerr << "\033[1;31m";
}

inline void terminal_exit_error()
{
  std::cerr << "\033[0m";
}