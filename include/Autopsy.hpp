#pragma once
#include "Warden.hpp"
#include <cstdint>
#include <sched.h>
#include <string>
#include <sys/types.h>

namespace psy {
struct Sample {
  pid_t pid = 0;
  std::uint64_t syscall = 0;
  std::string target = "not found";
  sec::Group group;
  bool is_allowed;
};
} // namespace psy
