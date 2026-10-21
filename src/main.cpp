#include <csignal>
#include <cstdlib>
#include <iostream>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

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

  while (true) { // Interception loop
    waitpid(pid, &status, 0);

    if (WIFEXITED(status) || WIFSIGNALED(status)) {
      break;
    }
    if (!WIFSTOPPED(status)) {
      continue;
    }

    int signal = WSTOPSIG(status);

    // TODO replace this code bellow with signals management
    if (signal == (SIGTRAP | 0x80)) {
      std::cout << "ptrace signal" << std::endl;
    } else {
      std::cout << "other signal" << std::endl;
    }

    ptrace(PTRACE_SYSCALL, pid, nullptr, nullptr);
  }

  std::cout << "Child process terminated" << std::endl;
  return 0;
}
