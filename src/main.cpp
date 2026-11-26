#include "Autopsy.hpp"
#include "PosixUtils.hpp"
#include "SandUtils.hpp"
#include "Tracer.hpp"
#include "Warden.hpp"
#include <iostream>
#include <sched.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  int i = 1;
  snd::Flags flags(snd::argsToFlags(i, argc, argv));
  std::cout << flags.toString() << std::endl;

  Tracer tracer(psx::launchProcess(argv[i], argv + i, flags));
  auto print = [](psy::Sample s) -> void {
    std::cout << sec::group_toString(s.group) << std::endl;
  };
  tracer.run(print);
  return 0;
}
