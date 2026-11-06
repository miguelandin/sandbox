#pragma once
#include "KernelAbi.hpp"
#include <cstdint>
#include <linux/limits.h>
#include <string>

// Functions to make your life easier by dealing with POSIX
namespace psx {

// reads chars from RAM directions
std::string readStringFromProcess(pid_t pid, std::uint64_t address,
                                  std::size_t max_len = PATH_MAX);

// launch a stopped process, ready for tracing
pid_t launchProcess(const char *filename, char *const argv[]);

inline void resumeProcess(pid_t pid) {
  ptrace(PTRACE_SYSCALL, pid, nullptr, nullptr);
}
} // namespace psx
