#include "PosixUtils.hpp"
#include "Tracer.hpp"
#include <iostream>
#include <sched.h>

int main(int argc, char *argv[]) {
  auto nmflags =
      CLONE_NEWNET | CLONE_NEWUSER | CLONE_NEWNS | CLONE_NEWPID | CLONE_NEWTIME;
  int i = 1;

  while (i < argc) {
    auto flag = argv[i];
    if (flag[0] != '-') {
      break;
    }

    switch (flag[1]) {
    case 'i': {
      nmflags = nmflags ^ CLONE_NEWNET;
      break;
    }
    case 'r': {
    case 'w': {
      nmflags = nmflags ^ CLONE_NEWNS;
      break;
    }
    }
    }

    i++;
  }

  Tracer tracer(psx::launchProcess(argv[i], argv + i, nmflags));
  auto print = [](std::string s) -> void { std::cout << s << std::endl; };
  tracer.run(print);
  return 0;
}
