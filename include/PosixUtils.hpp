#pragma once
#include "KernelAbi.hpp"
#include "SandUtils.hpp"
#include <cstdint>
#include <fcntl.h>
#include <filesystem>
#include <linux/limits.h>
#include <string>
#include <sys/mount.h>
#include <sys/ptrace.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// Functions to make your life easier by dealing with POSIX
namespace psx {

constexpr std::size_t MAX_EXEC_ARGS = 16;

inline constexpr auto PROTECTED_DIRS =
    std::to_array<std::string_view>({"/home", "/root", "/run/user"});

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

// allow ICMP
bool allowSocketsICMP();

// make current mount blind to other mounts
bool isolateMnt();

// hides dirs with important data by creating tmpfs mounts on top of them
bool coverUserDirs(const char *dir);

// mount a tmpfs dir at top of the path (user realpath)
bool mountTmpfsDir(const char *path);

// mounts tmpfs dirs at /root, /home, /run/user
bool mountTmpfsDirs();

// cretes a enviroment where only bin is visible in tmpfs mount
bool mountBinEnv(const std::filesystem::path &binPath, int originalFd);

// changes workDir, in case it fails with workDir, it tries with /
bool changeWorkDir(const std::filesystem::path &workDir);

// returns true if 'path' is 'dir' itself or any entry below it (doesnt's work
// with symlinks)
bool isInside(const std::filesystem::path &path,
              const std::filesystem::path &dir);

// returns tries if 'path' is inside protected dirs
bool isProtected(const std::filesystem::path &path);

// convert arg sysInfo arg to int
constexpr int asFd(std::uint64_t arg) { return static_cast<int>(arg); }

// reads chars from RAM directions
std::string readStringFromProcess(pid_t pid, std::uint64_t address,
                                  std::size_t max_len = PATH_MAX);

// identify family and type of a socket() call
std::string describeSocket(std::uint64_t domain, std::uint64_t type);

// Resolve a /proc/<pid>/<entry> magic link (fd/N, cwd...) to its target
std::string readProcLink(pid_t pid, const std::string &entry);

// What an open file descriptor of the tracee points to
std::string fdTarget(pid_t pid, int fd);

// Turn a path relative to dirfd (or to the cwd) into an absolute one
std::string resolveAt(pid_t pid, int dirfd, const std::string &path);

// Path passed as a plain pointer argument, relative to the cwd
inline std::string pathArg(pid_t pid, std::uint64_t address) {
  return resolveAt(pid, AT_FDCWD, psx::readStringFromProcess(pid, address));
}

// Path passed as a (dirfd, pointer) pair, as in every *at() syscall
inline std::string pathAtArg(pid_t pid, std::uint64_t dirfd,
                             std::uint64_t address) {
  return resolveAt(pid, asFd(dirfd), psx::readStringFromProcess(pid, address));
}

// File descriptor argument
inline std::string fdArg(pid_t pid, std::uint64_t fd) {
  return fdTarget(pid, asFd(fd));
}

// Copy raw bytes out of the tracee, returns how many could be read.
std::size_t readBytesFromProcess(pid_t pid, std::uint64_t address, void *out,
                                 std::size_t size);

// Read the NULL-terminated argv array of an exec call
std::string execArgs(pid_t pid, std::uint64_t argvAddress);

// Human readable form of a socket address (IPv4, IPv6 or Unix)
std::string formatSockaddr(const sockaddr_storage &storage, std::size_t len);
} // namespace psx
