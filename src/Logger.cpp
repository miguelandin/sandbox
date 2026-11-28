#include "Logger.hpp"
#include "SandUtils.hpp"
#include <optional>
#include <string_view>
#include <sys/syslog.h>

Logger::Logger(bool silent, std::string_view out) : silent(silent) {
  if (!out.empty()) {
    outFile.emplace(out.data());
  }
  openlog("sandbox", LOG_PID, LOG_USER);
}

Logger::Logger(const snd::Flags &f) : Logger(f.silent, f.saveFile) {}

Logger::~Logger() { closelog(); }
