#pragma once
#include <cstdint>
#include <sys/user.h>

// Interface for Arch depending functions
class ArchHandler {
public:

  // Extract a copy of the syscall id
  virtual std::uint64_t getSysCallNumber(const user_regs_struct &regs) = 0;

  virtual ~ArchHandler() = default;
};
