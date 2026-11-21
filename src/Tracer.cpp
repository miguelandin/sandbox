#include "Tracer.hpp"

std::string Tracer::manageSyscall(pid_t pid) {
  std::string res = std::to_string(m_sysInfo.entry.nr) + " ";

  switch (m_sysInfo.entry.nr) {
  case SYS_openat:
    res.append("FILE OPENED\n");
    res.append("FILE: " +
               psx::readStringFromProcess(pid, m_sysInfo.entry.args[1]));
    break;
  case SYS_open: {
    res.append("FILE OPENED\n");
    res.append("FILE: " +
               psx::readStringFromProcess(pid, m_sysInfo.entry.args[0]));
    break;
  }
  case SYS_connect: {
    res.append("NETWORK CONNECTION");
    break;
  }
  case SYS_execve: {
    res.append("BINARY EXECUTION\n");
    break;
  }
  default: {
    break;
  }
  }
  return res;
}
