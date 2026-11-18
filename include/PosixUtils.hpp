#pragma once
#include "KernelAbi.hpp"
#include <cstdint>
#include <linux/limits.h>
#include <string>

// Functions to make your life easier by dealing with POSIX
namespace psx {

// Bit that returns ptrace when it intercepts a sys call
constexpr int TRACE_IS_GOOD_BIT = 0x80;

// reads chars from RAM directions
std::string readStringFromProcess(pid_t pid, std::uint64_t address,
                                  std::size_t max_len = PATH_MAX);

// launch a stopped process, ready for tracing
pid_t launchProcess(const char *filename, char *const argv[], int nmFlags);

inline void resumeProcess(pid_t pid, int signal = 0) {
  ptrace(PTRACE_SYSCALL, pid, nullptr, reinterpret_cast<void *>(signal));
}

// every possible meaning of a waitpid status for a traced process
enum class StopKind { Ended, PtraceEvent, SyscallStop, SignalDelivery };

// return a StopKind value depending of the status of the process event
StopKind classifyStop(int status);
} // namespace psx
