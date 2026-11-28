#pragma once
#include <format>
#include <string_view>
#include <unistd.h>

namespace clr {
inline const bool enabled = isatty(STDERR_FILENO);

enum class Color { RED, GREEN, MAGENTA, CYAN };
constexpr std::string_view RESET = "\033[0m";

constexpr std::string_view code(Color c) noexcept {
  switch (c) {
  case Color::RED:
    return "\033[1;31m";
  case Color::GREEN:
    return "\033[1;32m";
  case Color::MAGENTA:
    return "\033[1;35m";
  case Color::CYAN:
    return "\033[1;36m";
  }
  return "";
}

inline std::string paint(std::string_view text, Color c) {
  if (!enabled) {
    return std::string(text);
  }

  return std::format("{}{}{}", code(c), text, RESET);
}
} // namespace clr
