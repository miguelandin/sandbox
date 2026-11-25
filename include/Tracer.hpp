#pragma once
#include "KernelAbi.hpp"
#include <PosixUtils.hpp>
#include <cerrno>
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
  pid_t m_pid;
  struct sandbox_ptrace_syscall_info m_sysInfo;

  std::string manageSyscall(pid_t pid);

public:
  Tracer(pid_t pid) : m_pid(pid) {
    ptrace(PTRACE_SEIZE, pid, nullptr, m_options);
  }

  template <typename... Handlers>
    requires(std::invocable<Handlers, std::string> && ...)
  void run(Handlers... handlers) {
    psx::resumeProcess(m_pid);

    while (true) {
      pid_t pid = waitpid(-1, &m_status, __WALL);

      if (pid == -1) {
        if (errno == EINTR) {
          continue;
        }
        if (errno == ECHILD) {
          break;
        }
        break;
      }

      int signal = 0;

      switch (psx::classifyStop(m_status)) {
      case psx::StopKind::Ended: {
        // TODO report exit code or the killing signal
        continue;
      }
      case psx::StopKind::PtraceEvent: {
        // TODO report PTRACE_GETEVENTMSG into u_long to log which pid was
        // created
        break;
      }
      case psx::StopKind::SeccompEvent: {
        if (ptrace(PTRACE_GET_SYSCALL_INFO, pid, sizeof(m_sysInfo),
                   &m_sysInfo) == -1) {
          break;
        }
        if (m_sysInfo.op != PTRACE_SYSCALL_INFO_ENTRY) {
          break;
        }
        (handlers(manageSyscall(pid)), ...);
        break;
      }
      case psx::StopKind::SignalDelivery: {
        signal = WSTOPSIG(m_status);
        break;
      }
      }
      psx::resumeProcess(pid, signal);
    }
  }
};
