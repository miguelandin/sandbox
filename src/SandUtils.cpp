#include "SandUtils.hpp"
#include <cstdint>
#include <cstring>
#include <format>
#include <sched.h>
#include <string_view>

snd::Flags::Flags(Group g, bool s, bool h, std::string_view o)
    : network(has(g, Group::NETWORK)), read(has(g, Group::READ)),
      write(has(g, Group::WRITE)), user(has(g, Group::USER)),
      exec(has(g, Group::EXEC)), silent(s), help(h), saveFile(o), groups(g) {
  if (!network) {
    nmflags |= CLONE_NEWNET;
  }
  if (!write) {
    nmflags |= CLONE_NEWNS;
  }
  if (!read) {
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

snd::Flags snd::parseArgs(int &i, int argc, char *argv[]) {
  Group groups = Group::NONE;
  bool silent = false;
  bool help = false;
  std::string_view saveFile;

  for (; i < argc; i++) {
    auto flag = argv[i];

    if (flag[0] != '-') {
      break;
    }

    if (std::strcmp(flag, "--help") == 0) {
      help = true;
      continue;
    }

    int l = 1;
    while (flag[l] != '\0') {
      if (flag[l] == 'h') {
        help = true;
      } else if (flag[l] == 's') {
        silent = true;
      } else if (flag[l] == 'o' && i + 1 < argc) {
        saveFile = argv[++i];
        break;
      } else {
        groups |= letterToGroup(flag[l]);
      }
      l++;
    }
  }

  return Flags(groups, silent, help, saveFile);
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
  return std::format("network={},read={},write={},user={},exec={},silent={},"
                     "help={},save={},nmflags={}",
                     network, read, write, user, exec, silent, help,
                     !saveFile.empty(), nmflags);
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
