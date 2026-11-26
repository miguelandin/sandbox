#pragma once
#include "KernelAbi.hpp"
#include "SandUtils.hpp"
#include <cstdint>
#include <linux/limits.h>
#include <string>
#include <sys/mount.h>
#include <sys/ptrace.h>
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

// resume process with his original instrucction
inline bool resumeProcess(pid_t pid, int signal = 0) {
  return ptrace(PTRACE_CONT, pid, nullptr, reinterpret_cast<void *>(signal)) !=
         -1;
}

// block stopped process by seccomp
bool blockSeccompProcess(pid_t pid);

// every possible meaning of a waitpid status for a traced process
enum class StopKind { Ended, PtraceEvent, SeccompEvent, SignalDelivery };

// return a StopKind value depending of the status of the process event
StopKind classifyStop(int status);

// change process uid and gid
bool mapUserNamespace(pid_t pid, uid_t uid, gid_t gid);

// write instructions in the process map
bool writeProcFile(const std::string &path, const std::string &content);

// make current mount blind to other mounts
bool isolateMnt();

// TODO mount() functions to complement seccomp limitations

} // namespace psx
