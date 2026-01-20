#pragma once

#include "generated/autoconf.hpp"

#ifdef CONFIG_TARGET_x86_64
#  include "arch/x86_64/frame.hpp"
using FrameImpl = arch::X86Frame;
#else
static_assert(false, "No target selected!");
#endif