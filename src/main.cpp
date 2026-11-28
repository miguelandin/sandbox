#include "PosixUtils.hpp"
#include "SandUtils.hpp"
#include "Tracer.hpp"
#include <iostream>
#include <sched.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int i = 1;
  snd::Flags flags = snd::parseArgs(i, argc, argv);

  if (flags.help) {
    std::cout << snd::USAGE;
    return 0;
  }

  Tracer tracer(psx::launchProcess(argv[i], argv + i, flags), flags);
  auto print = [](snd::Sample s) -> void {
    std::cout << snd::group_toString(s.group) << std::endl;
    std::cout << s.target << std::endl;
  };
  tracer.run(print);
  return 0;
}
