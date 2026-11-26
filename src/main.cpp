#include "PosixUtils.hpp"
#include "SandUtils.hpp"
#include "Tracer.hpp"
#include <iostream>
#include <sched.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int i = 1;
  snd::Flags flags(snd::argsToGroups(i, argc, argv));
  std::cout << flags.toString() << std::endl;

  Tracer tracer(psx::launchProcess(argv[i], argv + i, flags));
  auto print = [](snd::Sample s) -> void {
    std::cout << snd::group_toString(s.group) << std::endl;
  };
  tracer.run(print);
  return 0;
}
