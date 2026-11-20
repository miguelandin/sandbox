#include "PosixUtils.hpp"
#include <csignal>
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <sched.h>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

std::string psx::readStringFromProcess(pid_t pid, std::uint64_t address,
                                       std::size_t max_len) {
  constexpr std::size_t word_size = sizeof(long);

  std::size_t offset = address % word_size;
  auto addr = address - offset;

  std::string result;
  while (true) {
    errno = 0;
    long word =
        ptrace(PTRACE_PEEKDATA, pid, reinterpret_cast<void *>(addr), nullptr);
    if (errno != 0) {
      return "";
    }

    const char *bytes = reinterpret_cast<const char *>(&word);
    for (std::size_t i = offset; i < word_size; ++i) {
      if (bytes[i] == '\0') {
        return result;
      }
      if (result.size() == max_len) {
        return "";
      }
      result += bytes[i];
    }
    offset = 0;
    addr += word_size;
  }
}

pid_t psx::launchProcess(const char *binaryPath, char *const *arguments,
                         int nmFlags) {
  pid_t pid = fork();

  if (pid == 0) {
    if (unshare(nmFlags) == -1) {
      std::perror("unshare execution failed");
      _exit(1);
    }

    raise(SIGSTOP);
    execvp(binaryPath, arguments);

    std::perror("execve execution failed");
    _exit(127);
  }

  int status;
  waitpid(pid, &status, WUNTRACED);
  if (WIFSTOPPED(status)) {
    if ((nmFlags & CLONE_NEWUSER) != 0) {
      if (!psx::mapUserNamespace(pid, getuid(), getgid())) {
        std::perror("mapUserNamespace execution failed");
        _exit(1);
      }
    }
    return pid;
  }
  return -1;
}

psx::StopKind psx::classifyStop(int status) {
  if (WIFEXITED(status) || WIFSIGNALED(status)) {
    return psx::StopKind::Ended;
  }
  if ((status >> 16) != 0) {
    return psx::StopKind::PtraceEvent;
  }
  if (WSTOPSIG(status) == (SIGTRAP | psx::TRACE_IS_GOOD_BIT)) {
    return psx::StopKind::SyscallStop;
  }
  return psx::StopKind::SignalDelivery;
}

bool psx::mapUserNamespace(pid_t pid, uid_t uid, gid_t gid) {
  auto path = "/proc/" + std::to_string(pid);

  if (!psx::writeProcFile(path + "/setgroups", "deny")) {
    return false;
  }

  auto uMapContent = std::to_string(uid) + " " + std::to_string(uid) + " 1\n";
  auto gMapContent = std::to_string(gid) + " " + std::to_string(gid) + " 1\n";
  return psx::writeProcFile(path + "/uid_map", uMapContent) &&
         psx::writeProcFile(path + "/gid_map", gMapContent);
}

bool psx::writeProcFile(const std::string &path, const std::string &content) {
  int proc = open(path.c_str(), O_WRONLY);
  if (proc == -1) {
    std::perror(("open failed: " + path).c_str());
    return false;
  }

  auto written = write(proc, content.data(), content.size());
  if (written == -1) {
    std::perror(
        ("write failed: " + path + " with content: " + content).c_str());
    close(proc);
    return false;
  }

  close(proc);
  return true;
}
