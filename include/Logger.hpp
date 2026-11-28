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
  const std::string logMessage(const snd::Sample &s);
  const std::string jsonMessage(const snd::Sample &s);

public:
  Logger(bool silent, bool verbose, std::string_view filePath);
  Logger(const snd::Flags &flags);
  ~Logger();

  // call journal, print, and save by doing Logger(sample)
  void operator()(const snd::Sample &s);

  // write sandbox events in logs
  void log(const snd::Sample &s);

  // print in screem sandbox events
  void print(const snd::Sample &s);

  // save sandbox event in file
  void save(const snd::Sample &s);
};
