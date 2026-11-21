#include "PosixUtils.hpp"
#include "Tracer.hpp"
#include <iostream>
#include <sched.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  auto nmflags = CLONE_NEWNET | CLONE_NEWUSER | CLONE_NEWNS | CLONE_NEWPID;
  int i = 1;

  while (i < argc) {
    auto flag = argv[i];
    if (flag[0] != '-') {
      break;
    }

    switch (flag[1]) {
    case 'i': {
      nmflags ^= CLONE_NEWNET;
      break;
    }
    case 'r': {
    case 'w': {
      nmflags ^= CLONE_NEWNS;
      break;
    }
    }
    case 'p': {
      nmflags ^= CLONE_NEWPID;
      break;
    }
    case 'u': {
      nmflags ^= CLONE_NEWUSER;
    }
    }

    i++;
  }

  if (auto tmp = psx::makeTempDir(); tmp) {
    psx::makeSubDirs(*tmp, {"root", "upper", "work"});
  }

  Tracer tracer(psx::launchProcess(argv[i], argv + i, nmflags));
  auto print = [](std::string s) -> void { std::cout << s << std::endl; };
  tracer.run(print);
  return 0;
}
