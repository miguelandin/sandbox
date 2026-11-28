#pragma once

#include <cstdint>
#include <format>
#include <sched.h>
#include <string>
#include <string_view>

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
  Group group = Group::NONE;
  bool is_allowed = false;
};

// manage flags in a standard way
struct Flags {
  bool network = false;      // -n
  bool read = false;         // -r
  bool write = false;        // -w
  bool user = false;         // -u
  bool exec = false;         // -x
  bool silent = false;       // -s
  bool help = false;         // -h
  std::string_view saveFile; // -o

  Group groups = Group::NONE;
  int nmflags = 0;

  Flags() = default;
  Flags(Group groups, bool s, bool h, std::string_view o);
  Flags(Group groups) : Flags(groups, false, false, {}) {}
  std::string toString() const;
};

// convert argv into Flags options
Group argsToGroups(int &i, int argc, char *argv[]);

// convert argv into Flags options
Flags parseArgs(int &i, int argc, char *arv[]);

// convert char into Flag option bit
Group letterToGroup(char letter);

inline constexpr std::string_view USAGE =
    R"(Usage: sandbox [options] <binary> [args...]

Run a binary in an isolated, traced environment.
Everything is denied by default; flags grant permissions.

Permissions:
  -n          allow network access
  -r          allow reading files
  -w          allow writing in files
  -x          allow executing bins
  -u          allow executing by current user
  -A          allow everything

Options:
  -s          silent: do not print events to the terminal
  -o <file>   also save events to <file>
  -h, --help  show this help and exit
)";

} // namespace snd

template <>
struct std::formatter<snd::Group> : std::formatter<std::string_view> {
  auto format(snd::Group group, std::format_context &ctx) const {
    return std::formatter<std::string_view>::format(group_toString(group), ctx);
  }
};
