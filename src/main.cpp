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
    std::cout << "Executing: " << argv[i] << std::endl;
    execve(argv[i], argv + i, envp);
    std::perror("execve");
    _exit(127);
  } else {
    std::cout << "Im the parent" << std::endl;
  }
}
