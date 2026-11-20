#include "PosixUtils.hpp"
#include "Tracer.hpp"
#include <iostream>
#include <sched.h>

int main(int argc, char *argv[]) {
  int i = 1;

  while (i < argc) {
    if (argv[i][0] != '-') {
      break;
    }
    // TODO manage flags here
    i++;
  }

  auto nmflags = CLONE_NEWNET | CLONE_NEWUSER;
  Tracer tracer(psx::launchProcess(argv[i], argv + i, nmflags));
  auto print = [](std::string s) -> void { std::cout << s << std::endl; };
  tracer.run(print);
  return 0;
}
