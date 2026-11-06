#include "PosixUtils.hpp"
#include <csignal>
#include <sys/wait.h>
#include <vector>

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

pid_t psx::launchProcess(const char *binaryPath, char *const *arguments) {
  std::vector<const char *> envp = {"PATH=/usr/bin", "HOME=/tmp", nullptr};

  pid_t pid = fork();

  if (pid == 0) {
    raise(SIGSTOP);
    execve(binaryPath, arguments, const_cast<char *const *>(envp.data()));

    std::perror("execve execution failed");
    _exit(127);
  }

  int status;
  waitpid(pid, &status, WUNTRACED);
  if (WIFSTOPPED(status)) {
    return pid;
  }
  return -1;
}
