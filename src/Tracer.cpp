#include "Tracer.hpp"
#include "PosixUtils.hpp"
#include "SandUtils.hpp"
#include <algorithm>
#include <format>
#include <sched.h>
#include <sys/socket.h>

snd::Sample Tracer::manageSyscall() {
  snd::Sample sample;
  sample.pid = m_cPid;
  sample.syscall = m_sysInfo.seccomp.nr;
  sample.group = static_cast<snd::Group>(m_sysInfo.seccomp.ret_data);
  sample.target = getTarget(sample.group);
  sample.is_allowed =
      isLaunch(sample.group) || snd::has(m_groups, sample.group);

  if (!sample.is_allowed) {
    psx::blockSeccompProcess(m_cPid);
  }

  return sample;
}

bool Tracer::isLaunch(snd::Group sampleGroup) {
  return !m_launched && (m_cPid == m_pid) && sampleGroup == snd::Group::EXEC;
}

std::string Tracer::getTarget(snd::Group group) {
  switch (group) {
  case snd::Group::READ | snd::Group::WRITE:
    return readWriteCall();
  case snd::Group::READ:
    return readCall();
  case snd::Group::WRITE:
    return writeCall();
  case snd::Group::EXEC:
    return execCall();
  case snd::Group::NETWORK:
    return networkCall();
  default:
    return defaultCall();
  }
}

std::string Tracer::networkCall() {
  const auto &args = m_sysInfo.seccomp.args;

  switch (m_sysInfo.seccomp.nr) {
  case SYS_socket:
    return psx::describeSocket(args[0], args[1]);
  case SYS_connect:
  case SYS_bind: {
    sockaddr_storage storage{};
    auto len =
        std::min<std::size_t>(static_cast<socklen_t>(args[2]), sizeof(storage));
    return psx::formatSockaddr(
        storage, psx::readBytesFromProcess(m_cPid, args[1], &storage, len));
  }
  case SYS_sendto: {
    if (args[4] == 0) {
      return psx::fdArg(m_cPid, args[0]);
    }
    sockaddr_storage storage{};
    auto len =
        std::min<std::size_t>(static_cast<socklen_t>(args[5]), sizeof(storage));
    return psx::formatSockaddr(
        storage, psx::readBytesFromProcess(m_cPid, args[4], &storage, len));
  }
  case SYS_sendmsg:
  case SYS_sendmmsg: {
    msghdr header{};
    if (psx::readBytesFromProcess(m_cPid, args[1], &header, sizeof(header)) !=
            sizeof(header) ||
        header.msg_name == nullptr) {
      return psx::fdArg(m_cPid, args[0]);
    }
    sockaddr_storage storage{};
    auto len = std::min<std::size_t>(header.msg_namelen, sizeof(storage));
    return psx::formatSockaddr(
        storage, psx::readBytesFromProcess(
                     m_cPid, reinterpret_cast<std::uint64_t>(header.msg_name),
                     &storage, len));
  }
  case SYS_listen:
  case SYS_accept:
  case SYS_accept4:
    return psx::fdArg(m_cPid, args[0]);
  default:
    return defaultCall();
  }
}

std::string Tracer::readCall() {
  const auto &args = m_sysInfo.seccomp.args;

  switch (m_sysInfo.seccomp.nr) {
#ifdef SYS_open
  case SYS_open:
  case SYS_readlink:
    return psx::pathArg(m_cPid, args[0]);
  case SYS_getdents:
    return psx::fdArg(m_cPid, args[0]);
#endif
  case SYS_openat:
  case SYS_readlinkat:
    return psx::pathAtArg(m_cPid, args[0], args[1]);
  case SYS_read:
  case SYS_readv:
  case SYS_pread64:
  case SYS_preadv:
  case SYS_preadv2:
  case SYS_getdents64:
    return psx::fdArg(m_cPid, args[0]);
  default:
    return defaultCall();
  }
}

std::string Tracer::writeCall() {
  const auto &args = m_sysInfo.seccomp.args;

  switch (m_sysInfo.seccomp.nr) {
#ifdef SYS_open
  case SYS_open:
  case SYS_creat:
  case SYS_unlink:
  case SYS_rmdir:
  case SYS_mkdir:
  case SYS_mknod:
  case SYS_chmod:
  case SYS_chown:
  case SYS_lchown:
  case SYS_utime:
  case SYS_utimes:
    return psx::pathArg(m_cPid, args[0]);
  case SYS_rename:
  case SYS_link:
    return psx::pathArg(m_cPid, args[0]) + " -> " +
           psx::pathArg(m_cPid, args[1]);
  case SYS_symlink:
    return std::format("{} (symlink to {})", psx::pathArg(m_cPid, args[1]),
                       psx::readStringFromProcess(m_cPid, args[0]));
  case SYS_futimesat:
    return psx::pathAtArg(m_cPid, args[0], args[1]);
#endif
  case SYS_truncate:
  case SYS_setxattr:
  case SYS_lsetxattr:
  case SYS_removexattr:
  case SYS_lremovexattr:
    return psx::pathArg(m_cPid, args[0]);
  case SYS_openat:
  case SYS_unlinkat:
  case SYS_mkdirat:
  case SYS_mknodat:
  case SYS_fchownat:
  case SYS_fchmodat:
#ifdef SYS_fchmodat2
  case SYS_fchmodat2:
#endif
  case SYS_utimensat:
    return psx::pathAtArg(m_cPid, args[0], args[1]);
#ifdef SYS_renameat
  case SYS_renameat:
#endif
  case SYS_renameat2:
  case SYS_linkat:
    return psx::pathAtArg(m_cPid, args[0], args[1]) + " -> " +
           psx::pathAtArg(m_cPid, args[2], args[3]);
  case SYS_symlinkat:
    // (target, newdirfd, linkpath)
    return std::format("{} (symlink to {})",
                       psx::pathAtArg(m_cPid, args[1], args[2]),
                       psx::readStringFromProcess(m_cPid, args[0]));
  case SYS_write:
  case SYS_writev:
  case SYS_pwrite64:
  case SYS_pwritev:
  case SYS_pwritev2:
  case SYS_ftruncate:
  case SYS_fallocate:
  case SYS_fchmod:
  case SYS_fchown:
  case SYS_fsetxattr:
  case SYS_fremovexattr:
    return psx::fdArg(m_cPid, args[0]);
  case SYS_sendfile:
    return psx::fdArg(m_cPid, args[1]) + " -> " + psx::fdArg(m_cPid, args[0]);
  case SYS_copy_file_range:
  case SYS_splice:
    return psx::fdArg(m_cPid, args[0]) + " -> " + psx::fdArg(m_cPid, args[2]);
  default:
    return defaultCall();
  }
}

std::string Tracer::readWriteCall() { return readCall(); }

std::string Tracer::execCall() {
  const auto &args = m_sysInfo.seccomp.args;

  switch (m_sysInfo.seccomp.nr) {
  case SYS_execve:
    return std::format("{} argv:{}", psx::pathArg(m_cPid, args[0]),
                       psx::execArgs(m_cPid, args[1]));
  case SYS_execveat:
    return std::format("{} argv:{}", psx::pathAtArg(m_cPid, args[0], args[1]),
                       psx::execArgs(m_cPid, args[2]));
  default:
    return defaultCall();
  }
}

std::string Tracer::defaultCall() { return "syscall with no know target"; }
