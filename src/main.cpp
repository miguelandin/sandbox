#include <cstdlib>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Usage: " << argv[0] << "/<binary_route>" << std::endl;
    return 1;
  }

  std::cout << "[Sandbox] initializing controlled environment..." << std::endl;

  pid_t pid = fork();
  if (pid < 0) {
    std::cerr << "[Sandbox-Error] Failed creating fork()" << std::endl;
    return 1;
  } else if (pid == 0) {
    std::cout << "[Sandbox] binary executed with PID: " << getpid()
              << std::endl;

    if (execve(argv[1], argv + 1, nullptr) == -1) {
      std::cerr << "[Sandbox-Error] Failed to execute: " << argv[1]
                << std::endl;
    }
  } else {
    int status;
    waitpid(pid, &status, 0);

    if (WIFEXITED(status)) {
      std::cout << "[Sandbox] process ended with exit code: "
                << WEXITSTATUS(status) << std::endl;
    } else {
      std::cout << "[Sandbox] process ended abnormally" << std::endl;
    }
  }
}
