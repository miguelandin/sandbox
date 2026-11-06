#pragma once
#include <cstdint>
#include <sys/ptrace.h>

#ifndef PTRACE_GET_SYSCALL_INFO
#define PTRACE_GET_SYSCALL_INFO 0x420e
#endif

#ifndef PTRACE_SET_SYSCALL_INFO
#define PTRACE_SET_SYSCALL_INFO 0x4212
#endif

#ifndef PTRACE_SYSCALL_INFO_NONE
#define PTRACE_SYSCALL_INFO_NONE 0
#endif

#ifndef PTRACE_SYSCALL_INFO_ENTRY
#define PTRACE_SYSCALL_INFO_ENTRY 1
#endif

#ifndef PTRACE_SYSCALL_INFO_EXIT
#define PTRACE_SYSCALL_INFO_EXIT 2
#endif

#ifndef PTRACE_SYSCALL_INFO_SECCOMP
#define PTRACE_SYSCALL_INFO_SECCOMP 3
#endif

/* PTRACE_SYSCALL_INFO_* */
struct sandbox_ptrace_syscall_info {
  std::uint8_t op;
  std::uint8_t reserved;
  std::uint16_t flags;
  std::uint32_t arch;
  std::uint64_t instruction_pointer;
  std::uint64_t stack_pointer;
  union {
    struct {
      std::uint64_t nr;
      std::uint64_t args[6];
    } entry;
    struct {
      std::uint64_t rval;
      std::uint8_t is_error;
    } exit;
    struct {
      std::uint64_t nr;
      std::uint64_t args[6];
      std::uint32_t ret_data;
      std::uint32_t reserved2;
    } seccomp;
  };
};
