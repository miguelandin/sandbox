#include "Logger.hpp"
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
  Logger logger(flags);
  Tracer tracer(psx::launchProcess(argv[i], argv + i, flags), flags);
  tracer.run(std::ref(logger));
  return 0;
}
