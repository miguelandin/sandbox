#include "PosixUtils.hpp"
#include "Warden.hpp"
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <format>
#include <sched.h>
#include <string>
#include <sys/mount.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
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
    if (errno) {
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
                         const snd::Flags &flags) {

  pid_t pid = fork();

  if (pid == 0) {
    if (unshare(flags.nmflags) == -1) {
      std::perror("unshare execution failed");
      _exit(1);
    }

    raise(SIGSTOP);

    if (!flags.write) {
      if (!isolateMnt()) {
        _exit(1);
      }
    }

    sec::loadSeccomp();

    execvp(binaryPath, arguments);
    std::perror("execve execution failed");
    _exit(127);
  }

  int status;
  waitpid(pid, &status, WUNTRACED);
  if (WIFSTOPPED(status)) {
    if (!flags.user) {
      if (!psx::mapUserNamespace(pid, getuid(), getgid())) {
        std::exit(1);
      }
    }

    return pid;
  }
  return -1;
}

psx::StopKind psx::classifyStop(int status) {

  if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP &&
      ((status >> 16) == PTRACE_EVENT_SECCOMP)) {
    return psx::StopKind::SeccompEvent;
  }
  if (WIFEXITED(status) || WIFSIGNALED(status)) {
    return psx::StopKind::Ended;
  }
  if (status >> 16) {
    return psx::StopKind::PtraceEvent;
  }
  return psx::StopKind::SignalDelivery;
}

bool psx::mapUserNamespace(pid_t pid, uid_t uid, gid_t gid) {
  auto path = std::format("/proc/{}", pid);

  if (!psx::writeProcFile(std::format("{}/setgroups", path), "deny")) {
    std::perror("mapUserNamespace execution failed");
    return false;
  }

  auto uMapContent = std::format("{} {} 1\n", uid, uid);
  auto gMapContent = std::format("{} {} 1\n", gid, gid);
  return psx::writeProcFile(std::format("{}/uid_map", path), uMapContent) &&
         psx::writeProcFile(std::format("{}/gid_map", path), gMapContent);
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
        (std::format("write failed: {}, with content: {}", path, content)
             .c_str()));
    close(proc);
    return false;
  }

  close(proc);
  return true;
}

bool psx::isolateMnt() {
  if (mount(nullptr, "/", nullptr, MS_REC | MS_PRIVATE, nullptr) != 0) {
    std::perror("isolateMnt failed");
    return false;
  }
  return true;
}
