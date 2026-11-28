#pragma once
#include "SandUtils.hpp"
#include <fstream>
#include <optional>
#include <string_view>
#include <syslog.h>

class Logger {
  bool silent;
  bool verbose;
  std::optional<std::ofstream> outFile;

  const std::string paintedMessage(const snd::Sample &s);
  const std::string message(const snd::Sample &s);

public:
  Logger(bool silent, bool verbose, std::string_view filePath);
  Logger(const snd::Flags &flags);
  ~Logger();

  // call journal, print, and save by doing Logger(sample)
  void operator()(const snd::Sample &s);

  // write sandbox events in logs
  void log(std::string_view message);

  // print in screem sandbox events
  void print(std::string_view message);

  // save sandbox event in file
  void save(std::string_view message);
};
