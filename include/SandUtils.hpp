#pragma once

#include "Warden.hpp"
#include <cstdint>
#include <sched.h>
#include <string>

// main quality of life
namespace snd {
constexpr std::uint8_t NETWORK = 1u << 0;
constexpr std::uint8_t READ = 1u << 1;
constexpr std::uint8_t WRITE = 1u << 2;
constexpr std::uint8_t USER = 1u << 3;
constexpr std::uint8_t EXEC = 1u << 4;

// manage flags in a standard way
class Flags {
public:
  bool network = false; // -n
  bool read = false;    // -r
  bool write = false;   // -w
  bool user = false;    // -u
  bool exec = false;    // -x
  sec::Group allowedGroups; //TODO
  int nmflags = 0;

  Flags(std::uint8_t options = 0);
  std::string toString() const;
};

// convert argv into Flags options
std::uint8_t argsToFlags(int &i, int argc, char *argv[]);

// convert char into Flag option bit
std::uint8_t letterToBit(char letter);

} // namespace snd
