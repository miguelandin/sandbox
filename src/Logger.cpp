#include "Logger.hpp"
#include "Color.hpp"
#include "SandUtils.hpp"
#include <format>
#include <iostream>
#include <optional>
#include <string_view>
#include <sys/syslog.h>

Logger::Logger(bool silent, bool verbose, std::string_view out)
    : silent(silent), verbose(verbose) {
  if (!out.empty()) {
    outFile.emplace(out.data());
  }
  openlog("sandbox", LOG_PID, LOG_USER);
}

Logger::Logger(const snd::Flags &f) : Logger(f.silent, f.verbose, f.saveFile) {}

Logger::~Logger() { closelog(); }

void Logger::operator()(const snd::Sample &s) {
  auto plainMessage = message(s);
  if (!silent) {
    if (!verbose) {
      if (!s.is_allowed) {
        print(paintedMessage(s));
      }
    } else {
      print(paintedMessage(s));
    }
  }

  /* if (outFile) {
     save(plainMessage);
   }

   log(plainMessage);*/
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

const std::string Logger::message(const snd::Sample &s) {
  return std::format("pid={} group={} syscall={} allowed={} target={}", s.pid,
                     s.group, s.syscall, s.is_allowed, s.target);
}

void Logger::print(std::string_view msg) { std::cerr << msg << "\n"; }
