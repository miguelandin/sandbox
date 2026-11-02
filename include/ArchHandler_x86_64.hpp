#pragma once
#include <ArchHandler.hpp>
#include <cstdint>
#include <sys/user.h>

// Functions for Arch x86_64
class ArchHandler_x86_64 : public ArchHandler {
public:
  std::uint64_t getSysCallNumber(const user_regs_struct &regs) override;
};
