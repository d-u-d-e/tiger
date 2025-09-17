#pragma once
#include <generated/config.hpp>

#ifdef CONFIG_TARGET_x86_64
  #include "arch/x86-64/frame.hpp"
  #include "arch/x86-64/isel.hpp"
#else
#error "Please select a supported target"
#endif