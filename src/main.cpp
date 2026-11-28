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
    std::cerr << snd::USAGE;
    return 0;
  }

  if(i>=argc){
    std::cerr<<snd::USAGE;
    return 1;
  }

  Logger logger(flags);
  Tracer tracer(psx::launchProcess(argv[i], argv + i, flags), flags, argv[i]);
  tracer.run(std::ref(logger));

  return 0;
}
