#pragma once
#include "KernelAbi.hpp"
#include "SandUtils.hpp"
#include <cstdint>
#include <format>
#include <initializer_list>
#include <linux/limits.h>
#include <optional>
#include <string>
#include <string_view>
#include <sys/mount.h>
#include <sys/types.h>
#include <unistd.h>

// Functions to make your life easier by dealing with POSIX
namespace psx {

// Bit that returns ptrace when it intercepts a sys call
constexpr int TRACE_IS_GOOD_BIT = 0x80;

// reads chars from RAM directions
std::string readStringFromProcess(pid_t pid, std::uint64_t address,
                                  std::size_t max_len = PATH_MAX);

// launch a stopped process, ready for tracing
pid_t launchProcess(const char *filename, char *const argv[],
                    const snd::Flags &flags);

inline void resumeProcess(pid_t pid, int signal = 0) {
  ptrace(PTRACE_SYSCALL, pid, nullptr, reinterpret_cast<void *>(signal));
}

// every possible meaning of a waitpid status for a traced process
enum class StopKind { Ended, PtraceEvent, SyscallStop, SignalDelivery };

// return a StopKind value depending of the status of the process event
StopKind classifyStop(int status);

// change process uid and gid
bool mapUserNamespace(pid_t pid, uid_t uid, gid_t gid);

// write instructions in the process map
bool writeProcFile(const std::string &path, const std::string &content);

// make current mount blind to other mounts
bool isolateMnt();

} // namespace psx
