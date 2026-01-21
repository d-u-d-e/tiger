#pragma once

#include "generated/autoconf.hpp"

#if CONFIG_TARGET_x86_64
#  include "arch/x86_64/frame.hpp"
#  include "arch/x86_64/generator.hpp"
using FrameImpl = arch::X86Frame;
using GeneratorImpl = arch::X86Generator;
#else
static_assert(false, "No target selected!");
#endif