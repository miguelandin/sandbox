#include "SandUtils.hpp"
#include <cstdint>
#include <format>
#include <sched.h>

snd::Flags::Flags(Group g)
    : network(has(g, Group::NETWORK)), read(has(g, Group::READ)),
      write(has(g, Group::WRITE)), user(has(g, Group::USER)),
      exec(has(g, Group::EXEC)), groups(g) {
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

snd::Group snd::argsToGroups(int &i, int argc, char *argv[]) {
  Group groups = Group::NONE;
  for (; i < argc; i++) {
    auto flag = argv[i];

    if (flag[0] != '-') {
      break;
    }

    int l = 1;
    while (flag[l] != '\0') {
      groups |= letterToGroup(flag[l]);
      l++;
    }
  }

  return groups;
}

snd::Group snd::letterToGroup(char letter) {
  switch (letter) {
  case 'n':
    return Group::NETWORK;
  case 'r':
    return Group::READ;
  case 'w':
    return Group::WRITE;
  case 'u':
    return Group::USER;
  case 'x':
    return Group::EXEC;
  case 'A':
    return static_cast<Group>(UINT8_MAX);
  default:
    std::perror(std::format("'{}' flag doesn't exist", letter).c_str());
    return Group::NONE;
  }
}

std::string snd::Flags::toString() const {
  return std::format("network={},read={},write={},user={},exec={},nmflags={}",
                     network, read, write, user, exec, nmflags);
}

std::string_view snd::group_toString(Group group) noexcept {
  switch (group) {
  case Group::NETWORK:
    return "network";
  case Group::READ:
    return "read";
  case Group::WRITE:
    return "write";
  case Group::READ | Group::WRITE:
    return "read/write";
  case Group::EXEC:
    return "execution";
  case Group::USER:
    return "user";
  default:
    return "NONE";
  }
}
