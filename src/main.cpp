#include "PosixUtils.hpp"
#include "SandUtils.hpp"
#include "Tracer.hpp"
#include <iostream>
#include <sched.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int i = 1;
  snd::Flags flags(snd::argsToFlags(i, argc, argv));
  // std::cout << flags.toString() << std::endl;

  if (auto tmp = psx::makeTempDir(); tmp) {
    psx::makeSubDirs(*tmp, {"root", "upper", "work"});
  }

  Tracer tracer(psx::launchProcess(argv[i], argv + i, flags));
  auto print = [](std::string s) -> void { std::cout << s << std::endl; };
  tracer.run(print);
  return 0;
}
