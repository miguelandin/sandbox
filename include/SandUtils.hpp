#pragma once

#include <cstdint>
#include <sched.h>
#include <string>

// main quality of life
namespace snd {
constexpr std::uint8_t NETWORK = 1u << 0;
constexpr std::uint8_t READ = 1u << 1;
constexpr std::uint8_t SWRITE = 1u << 2;
constexpr std::uint8_t FWRITE = 1u << 3;
constexpr std::uint8_t USER = 1u << 4;
constexpr std::uint8_t EXEC = 1u << 5;

class Flags {
public:
  bool network = false; // -n
  bool read = false;    // -r
  bool swrite = false;  // -w
  bool fwrite = false;  // -W
  bool user = false;    // -u
  bool exec = false;    // -e
  int nmflags = 0;

  Flags(std::uint8_t options = 0);
  std::string toString() const;
};

std::uint8_t argsToFlags(int &i, int argc, char *argv[]);

std::uint8_t letterToBit(char letter);

} // namespace snd
