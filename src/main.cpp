#include "PosixUtils.hpp"
#include "Tracer.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  int i = 1;

  while (i < argc) {
    if (argv[i][0] != '-') {
      break;
    }
    // TODO manage flags here
    i++;
  }

  Tracer tracer(psx::launchProcess(argv[i], argv + i));
  auto print = [](std::string s)->void{std::cout<<s<<std::endl;};
  tracer.run(print);
  return 0;
}
