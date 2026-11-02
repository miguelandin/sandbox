#include "ArchHandler_x86_64.hpp"

std::uint64_t
ArchHandler_x86_64::getSysCallNumber(const user_regs_struct &regs) {
  return regs.orig_rax;
};
