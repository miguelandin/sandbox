#include "Tracer.hpp"
#include "Autopsy.hpp"
#include "Warden.hpp"
#include <sched.h>

psy::Sample Tracer::manageSyscall(pid_t pid) {
  psy::Sample sample;
  sample.pid = pid;
  sample.syscall = m_sysInfo.seccomp.nr;
  sample.group = static_cast<sec::Group>(m_sysInfo.seccomp.ret_data);

  switch (sample.group) {
  case sec::Group::READ | sec::Group::WRITE:
    readWriteCall(pid, sample);
    break;
  case sec::Group::READ:
    readCall(pid, sample);
    break;
  case sec::Group::WRITE:
    writeCall(pid, sample);
    break;
  case sec::Group::EXEC:
    execCall(pid, sample);
    break;
  case sec::Group::NETWORK:
    networkCall(pid, sample);
    break;
  default:
    defaultCall(pid, sample);
    break;
  }
}

bool Tracer::networkCall(pid_t pid, psy::Sample &sample) {
  // TODO
}

bool Tracer::readCall(pid_t pid, psy::Sample &sample) {
  // TODO
}

bool Tracer::writeCall(pid_t pid, psy::Sample &sample) {
  // TODO
}

bool Tracer::readWriteCall(pid_t pid, psy::Sample &sample) {
  // TODO
}

bool Tracer::execCall(pid_t pid, psy::Sample &sample) {
  // TODO
}

bool Tracer::defaultCall(pid_t pid, psy::Sample &sample) {
  // TODO
}
/*switch (m_sysInfo.entry.nr) {
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
return res;*/
