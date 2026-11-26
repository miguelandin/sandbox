#pragma once

#include <cstdint>
#include <format>
#include <sched.h>
#include <string>

// main quality of life
namespace snd {

enum class Group : std::uint16_t {
  NONE = 0,
  NETWORK = 1,
  READ = 2,
  WRITE = 4,
  EXEC = 8,
  USER = 16,
  READ_WRITE = READ | WRITE
};

std::string_view group_toString(Group group) noexcept;

constexpr std::uint16_t raw(Group g) noexcept {
  return static_cast<std::uint16_t>(g);
}

constexpr Group operator|(snd::Group a, snd::Group b) noexcept {
  return static_cast<snd::Group>(raw(a) | raw(b));
}

constexpr Group operator&(snd::Group a, snd::Group b) noexcept {
  return static_cast<snd::Group>(raw(a) & raw(b));
}

constexpr Group operator|=(snd::Group &a, snd::Group b) noexcept {
  a = a | b;
  return a;
}

constexpr bool has(Group set, Group flag) noexcept {
  return (set & flag) == flag;
}

// analyzed process result
struct Sample {
  pid_t pid = 0;
  std::uint64_t syscall = 0;
  std::string target = "not found";
  Group group;
  bool is_allowed;
};

// manage flags in a standard way
struct Flags {
  bool network = false; // -n
  bool read = false;    // -r
  bool write = false;   // -w
  bool user = false;    // -u
  bool exec = false;    // -x
  Group groups;
  int nmflags = 0;

  Flags(Group groups = Group::NONE);
  std::string toString() const;
};

// convert argv into Flags options
Group argsToGroups(int &i, int argc, char *argv[]);

// convert char into Flag option bit
Group letterToGroup(char letter);

} // namespace snd

template <>
struct std::formatter<snd::Group> : std::formatter<std::string_view> {
  auto format(snd::Group group, std::format_context &ctx) const {
    return std::formatter<std::string_view>::format(group_toString(group), ctx);
  }
};
