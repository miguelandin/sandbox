#include "Logger.hpp"
#include "Color.hpp"
#include "SandUtils.hpp"
#include <chrono>
#include <format>
#include <iostream>
#include <optional>
#include <string_view>
#include <syslog.h>

Logger::Logger(bool silent, bool verbose, std::string_view out)
    : silent(silent), verbose(verbose) {
  if (!out.empty()) {
    outFile.emplace(std::string(out.data()), std::ios::app);
    if (!outFile->is_open()) {
      std::cerr << std::format("couldn't open out file at: {}", out);
    }
  }
  openlog(snd::APPNAME.data(), LOG_PID, LOG_USER);
}

Logger::Logger(const snd::Flags &f) : Logger(f.silent, f.verbose, f.saveFile) {}

Logger::~Logger() { closelog(); }

void Logger::operator()(const snd::Sample &s) {
  if (!silent && (verbose || !s.is_allowed)) {
    print(s);
  }
  save(s);
  log(s);
}

const std::string Logger::paintedMessage(const snd::Sample &s) {
  auto pid = clr::paint(std::to_string(s.pid), clr::Color::CYAN);
  auto group = clr::paint(snd::group_toString(s.group), clr::Color::MAGENTA);
  auto target = clr::paint(s.target, clr::Color::CYAN);
  auto allowed = s.is_allowed ? clr::paint("ALLOWED", clr::Color::GREEN)
                              : clr::paint("BLOCKED", clr::Color::RED);
  auto syscall = clr::paint(std::to_string(s.syscall), clr::Color::MAGENTA);

  return std::format("[{}] pid {} group {} syscall {} {} target {}",
                     clr::paint(snd::APPNAME, clr::Color::GREEN), pid, group,
                     syscall, allowed, target);
}

const std::string Logger::logMessage(const snd::Sample &s) {
  return std::format(
      "pid='{}' group='{}' syscall='{}' allowed='{}' target='{}'", s.pid,
      s.group, s.syscall, s.is_allowed, s.target);
}

const std::string Logger::jsonMessage(const snd::Sample &s) {
  auto now = std::chrono::floor<std::chrono::seconds>(
      std::chrono::system_clock::now());

  return std::format(
      R"({{"ts":"{:%FT%TZ}","pid":{},"nr":{},"group":"{}","allowed":{},"target":"{}"}})",
      now, s.pid, s.syscall, s.group, s.is_allowed, s.target);
}

void Logger::print(const snd::Sample &s) {
  std::cerr << paintedMessage(s) << "\n";
}

void Logger::save(const snd::Sample &s) {
  if (outFile) {
    *outFile << jsonMessage(s) << "\n";
    outFile->flush();
  }
}

void Logger::log(const snd::Sample &s) {
  auto prio = s.is_allowed ? LOG_INFO : LOG_WARNING;
  syslog(prio, "%s", logMessage(s).c_str());
}
