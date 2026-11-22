#include "SandUtils.hpp"
#include <cstdint>
#include <format>
#include <limits>
#include <sched.h>

snd::Flags::Flags(std::uint8_t o)
    : network(o & NETWORK), read(o & READ), write(o & WRITE), user(o & USER),
      exec(o & EXEC) {
  if (!network) {
    nmflags |= CLONE_NEWNET;
  }
  if (!write) {
    nmflags |= CLONE_NEWNS;
  }
  if (!user) {
    nmflags |= CLONE_NEWUSER;
  }
  if (!exec) {
    nmflags |= CLONE_NEWPID;
  }
}

std::uint8_t snd::argsToFlags(int &i, int argc, char *argv[]) {
  std::uint8_t flags = 0;
  for (; i < argc; i++) {
    auto flag = argv[i];

    if (flag[0] != '-') {
      break;
    }

    int l = 1;
    while (flag[l] != '\0') {
      flags |= letterToBit(flag[l]);
      l++;
    }
  }

  return flags;
}

std::uint8_t snd::letterToBit(char letter) {
  switch (letter) {
  case 'n':
    return NETWORK;
  case 'r':
    return READ;
  case 'w':
    return WRITE;
  case 'u':
    return USER;
  case 'x':
    return EXEC;
  case 'A':
    return UINT8_MAX;
  default:
    std::perror(std::format("'{}' flag doesn't exist", letter).c_str());
    return 0;
  }
}

std::string snd::Flags::toString() const {
  return std::format("network={},read={},write={},user={},exec={},nmflags={}",
                     network, read, write, user, exec, nmflags);
}
