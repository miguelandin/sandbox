#include <csignal>
#include <cstdlib>
#include <iostream>
#include <linux/limits.h>
#include <string>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <syscall.h>
#include <unistd.h>

// READ process c-string
std::string read_string_from_process(pid_t pid, unsigned long address,
                                   std::size_t max_len = PATH_MAX) {

  constexpr std::size_t word_size = sizeof(long);

  std::size_t offset = address % word_size;
  auto addr = address - offset;

  std::string result;
  while (true) {
    errno = 0;
    long word = ptrace(PTRACE_PEEKDATA, pid,
                       reinterpret_cast<void *>(addr), nullptr);
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

int main(int argc, char *argv[]) {

  std::string path = std::string("PATH=") + std::getenv("PATH");
  std::string home = std::string("HOME=/tmp");
  char *envp[] = {path.data(), home.data(), nullptr};
  int i = 1;

  while (i < argc) {
    if (argv[i][0] != '-') {
      break;
    }
    // TODO manage flags here
    i++;
  }

  pid_t pid = fork();

  if (pid == 0) {
    raise(SIGSTOP);
    execve(argv[i], argv + i, envp);

    std::perror("execve");
    _exit(127);
  }

  int status;
  waitpid(pid, &status, WUNTRACED);
  if (!WIFSTOPPED(status)) {
    std::cerr << "Child did not stop succesfully" << std::endl;
  }

  int options = PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEEXEC;
  ptrace(PTRACE_SEIZE, pid, nullptr, options);
  ptrace(PTRACE_SYSCALL, pid, nullptr, nullptr);

  bool manage = true;
  while (true) { // Interception loop
    waitpid(pid, &status, 0);

    if (WIFEXITED(status) || WIFSIGNALED(status)) {
      break;
    }
    if (!WIFSTOPPED(status)) {
      continue;
    }

    int signal = WSTOPSIG(status);

    if (signal == (SIGTRAP | 0x80)) {
      if (manage) {
        struct user_regs_struct regs;
        ptrace(PTRACE_GETREGS, pid, nullptr, &regs);
        auto syscall_id = regs.orig_rax;

        switch (syscall_id) {
        case SYS_open:
          std::cout << "FILE OPENED " << syscall_id << std::endl;
          std::cout << read_string_from_process(pid, regs.rdi) << std::endl;
          break;
        case SYS_openat:
          std::cout << "FILE OPENED " << syscall_id << std::endl;
          std::cout << read_string_from_process(pid, regs.rsi) << std::endl;
          break;
        case SYS_connect:
          std::cout << "NETWORK CONNECTION " << syscall_id << std::endl;
          break;
        case SYS_execve:
          std::cout << "BINARY EXECUTION " << syscall_id << std::endl;
          break;
        default:
          break;
        }
      }
      manage = !manage;
    }

    ptrace(PTRACE_SYSCALL, pid, nullptr, nullptr);
  }

  std::cout << "Child process terminated" << std::endl;
  return 0;
}
