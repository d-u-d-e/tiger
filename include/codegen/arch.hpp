#pragma once
#include <generated/autoconf.hpp>

// TODO: only one target must be set

#if CONFIG_TARGET_x86_64
  #include "arch/x86-64/frame.hpp"
  #include "arch/x86-64/isel.hpp"
  #include "arch/x86-64/helpers.hpp"
#else
#error "Please select a supported target"
#endif