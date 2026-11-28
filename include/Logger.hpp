#pragma once
#include "SandUtils.hpp"
#include <fstream>
#include <optional>
#include <syslog.h>

class Logger {
  bool silent;
  std::optional<std::ofstream> outFile;

public:
  Logger(bool silent, std::string_view filePath);
  Logger(const snd::Flags &flags);
  ~Logger();

  // call journal, print, and save by doing Logger(sample)
  void operator()(const snd::Sample &s);

  // write sandbox events in logs
  void journal(const snd::Sample &s);

  // print in screem sandbox events
  void print(const snd::Sample &s);

  // save sandbox event in file
  void save(const snd::Sample &s);
};
