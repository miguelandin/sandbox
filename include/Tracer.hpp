#pragma once
#include "KernelAbi.hpp"
#include "PosixUtils.hpp"
#include "SandUtils.hpp"
#include <cerrno>
#include <filesystem>
#include <string>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/wait.h>

class Tracer {
private:
  int m_options = PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEEXEC |
                  PTRACE_O_EXITKILL | PTRACE_O_TRACEFORK | PTRACE_O_TRACECLONE |
                  PTRACE_O_TRACEVFORK | PTRACE_O_TRACESECCOMP;
  int m_status;
  int m_signal;
  pid_t m_pid;
  pid_t m_cPid;
  snd::Group m_groups;
  bool m_launched = false;
  std::filesystem::path m_binPath;
  struct sandbox_ptrace_syscall_info m_sysInfo;

  snd::Sample manageSyscall();
  std::string networkCall();
  std::string readCall();
  std::string writeCall();
  std::string readWriteCall();
  std::string execCall();
  std::string defaultCall();
  std::string getTarget(snd::Group group);
  bool isLaunch(snd::Group sampleGroup);
  bool isLaunchBin(std::filesystem::path target);

public:
  Tracer(pid_t pid, snd::Group g) : m_pid(pid), m_groups(g) {
    ptrace(PTRACE_SEIZE, pid, nullptr, m_options);
  }

  Tracer(pid_t pid, snd::Flags f, const char *binPath) : Tracer(pid, f.groups) {
    std::error_code ec;
    m_binPath = std::filesystem::canonical(binPath, ec);
  }

  template <typename... Handlers>
    requires(std::invocable<Handlers, snd::Sample> && ...)
  void run(Handlers... handlers) {
    psx::resumeProcess(m_pid);

    while (true) {
      m_cPid = waitpid(-1, &m_status, __WALL);

      if (m_cPid == -1) {
        if (errno == EINTR) {
          continue;
        }
        if (errno == ECHILD) {
          break;
        }
        break;
      }

      m_signal = 0;

      switch (psx::classifyStop(m_status)) {
      case psx::StopKind::Ended: {
        // TODO report exit code or the killing signal
        continue;
      }
      case psx::StopKind::PtraceEvent: {
        if ((m_status >> 16) == PTRACE_EVENT_EXEC && m_cPid == m_pid) {
          m_launched = true;
        }
        // TODO report PTRACE_GETEVENTMSG into u_long to log which pid was
        // created
        break;
      }
      case psx::StopKind::SeccompEvent: {
        if (ptrace(PTRACE_GET_SYSCALL_INFO, m_cPid, sizeof(m_sysInfo),
                   &m_sysInfo) == -1) {
          break;
        }
        if (m_sysInfo.op != PTRACE_SYSCALL_INFO_SECCOMP) {
          break;
        }
        (handlers(manageSyscall()), ...);
        break;
      }
      case psx::StopKind::SignalDelivery: {
        m_signal = WSTOPSIG(m_status);
        break;
      }
      }
      psx::resumeProcess(m_cPid, m_signal);
    }
  }
};
