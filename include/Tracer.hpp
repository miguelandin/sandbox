#pragma once
#include "KernelAbi.hpp"
#include <PosixUtils.hpp>
#include <string>
#include <sys/syscall.h>
#include <sys/wait.h>

class Tracer {
private:
  int TRACE_IS_GOOD_BIT = 0x80;
  int m_options = PTRACE_O_TRACESYSGOOD | PTRACE_O_TRACEEXEC;
  int m_status;
  pid_t m_pid;
  struct sandbox_ptrace_syscall_info m_sysInfo;

public:
  Tracer(pid_t pid) : m_pid(pid) {
    ptrace(PTRACE_SEIZE, pid, nullptr, m_options);
  }

  template <typename... Handlers>
    requires(std::invocable<Handlers, std::string> && ...)
  void run(Handlers... handlers) {
    while (true) {
      psx::resumeProcess(m_pid);
      waitpid(m_pid, &m_status, 0);

      if (WIFEXITED(m_status) || WIFSIGNALED(m_status)) {
        break;
      }
      if (!WIFSTOPPED(m_status)) {
        continue;
      }
      if (WSTOPSIG(m_status) != (SIGTRAP | TRACE_IS_GOOD_BIT)) {
        continue;
      }
      if (ptrace(PTRACE_GET_SYSCALL_INFO, m_pid, sizeof(m_sysInfo),
                 &m_sysInfo) == -1) {
        continue;
      }
      if (m_sysInfo.op != PTRACE_SYSCALL_INFO_ENTRY) {
        continue;
      }

      std::string res = std::to_string(m_sysInfo.entry.nr);

      switch (m_sysInfo.entry.nr) {
      case SYS_openat:
        res.append(" FILE OPENED\n");
        res.append("FILE: " +
                   psx::readStringFromProcess(m_pid, m_sysInfo.entry.args[1]));
        break;
      case SYS_open:
        res.append(" FILE OPENED\n");
        res.append("FILE: " +
                   psx::readStringFromProcess(m_pid, m_sysInfo.entry.args[0]));
        break;
      case SYS_connect:
        res.append(" NETWORK CONNECTION");
        break;
      case SYS_execve:
        res.append("BINARY EXECUTION\n");
        break;
      default:
        break;
      }

      (handlers(res), ...);
    }
  }
};
