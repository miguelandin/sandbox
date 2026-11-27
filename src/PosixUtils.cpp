#include "PosixUtils.hpp"
#include "Warden.hpp"
#include <arpa/inet.h>
#include <array>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <format>
#include <iostream>
#include <netinet/in.h>
#include <sched.h>
#include <string>
#include <sys/mount.h>
#include <sys/ptrace.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <system_error>
#include <unistd.h>

pid_t psx::launchProcess(const char *binaryPath, char *const *arguments,
                         const snd::Flags &flags) {

  pid_t pid = fork();

  if (pid == 0) {
    if (unshare(flags.nmflags) == -1) {
      std::perror("unshare execution failed");
      _exit(1);
    }

    raise(SIGSTOP);

    if (!flags.network && !flags.user) {
      if (!allowSocketsICMP()) {
        _exit(1);
      }
    }

    if (!flags.write || !flags.read) {
      if (!isolateMnt()) {
        _exit(1);
      }
    }

    if (!flags.read) {
      if (!coverUserDirs(binaryPath)) {
        _exit(1);
      }
    }

    if (!sec::loadSeccomp()) {
      _exit(1);
    }

    execvp(binaryPath, arguments);
    std::perror("execve execution failed");
    _exit(127);
  }

  int status;
  waitpid(pid, &status, WUNTRACED);
  if (WIFSTOPPED(status)) {
    if (!flags.user) {
      if (!psx::mapUserNamespace(pid, getuid(), getgid())) {
        std::exit(1);
      }
    }

    return pid;
  }
  return -1;
}

psx::StopKind psx::classifyStop(int status) {

  if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP &&
      ((status >> 16) == PTRACE_EVENT_SECCOMP)) {
    return psx::StopKind::SeccompEvent;
  }
  if (WIFEXITED(status) || WIFSIGNALED(status)) {
    return psx::StopKind::Ended;
  }
  if (status >> 16) {
    return psx::StopKind::PtraceEvent;
  }
  return psx::StopKind::SignalDelivery;
}

bool psx::mapUserNamespace(pid_t pid, uid_t uid, gid_t gid) {
  auto path = std::format("/proc/{}", pid);

  if (!psx::writeProcFile(std::format("{}/setgroups", path), "deny")) {
    std::perror("mapUserNamespace execution failed");
    return false;
  }

  auto uMapContent = std::format("{} {} 1\n", uid, uid);
  auto gMapContent = std::format("{} {} 1\n", gid, gid);
  return psx::writeProcFile(std::format("{}/uid_map", path), uMapContent) &&
         psx::writeProcFile(std::format("{}/gid_map", path), gMapContent);
}

bool psx::writeProcFile(const std::string &path, const std::string &content) {
  int proc = open(path.c_str(), O_WRONLY);
  if (proc == -1) {
    std::perror(("open failed: " + path).c_str());
    return false;
  }

  auto written = write(proc, content.data(), content.size());
  if (written == -1) {
    std::perror(
        (std::format("write failed: {}, with content: {}", path, content)
             .c_str()));
    close(proc);
    return false;
  }

  close(proc);
  return true;
}

bool psx::allowSocketsICMP() {
  auto gid = getgid();
  return writeProcFile("/proc/sys/net/ipv4/ping_group_range",
                       std::format("{} {}\n", gid, gid));
}

bool psx::isolateMnt() {
  if (mount(nullptr, "/", nullptr, MS_REC | MS_PRIVATE, nullptr) != 0) {
    std::perror("isolateMnt failed");
    return false;
  }
  return true;
}

bool psx::coverUserDirs(const char *binPath) {
  std::error_code ec;
  auto pwd = std::filesystem::current_path(ec);
  auto realPath = std::filesystem::canonical(binPath, ec);
  bool isRealPath = !ec.value();
  bool isProtectedDir = isInside(realPath, "/home") ||
                        isInside(realPath, "/root") ||
                        isInside(realPath, "/run/user");

  int originalFd = -1;
  if (isRealPath && isProtectedDir) {
    // saves i-node of the binPath, when used, it dissappears (works even if the
    // dir is mounted on top)
    originalFd = open(realPath.c_str(), O_PATH | O_CLOEXEC);
    if (originalFd == -1) {
      std::perror(
          std::format("couldn't open bin at: {}", realPath.c_str()).c_str());
      return false;
    }
  }

  if (!mountTmpfsDirs()) {
    return false;
  }

  if (originalFd != -1) {
    bool mounted = mountBinEnv(realPath, originalFd);
    close(originalFd);
    if (!mounted) {
      return false;
    }
  }

  return changeWorkDir(pwd);
}

bool psx::mountBinEnv(const std::filesystem::path &binPath, int originalFd) {
  std::error_code ec;
  std::filesystem::create_directories(binPath.parent_path(), ec);
  if (ec) {
    std::cerr << ec.message();
    return false;
  }

  auto descriptor = open(binPath.c_str(), O_WRONLY | O_CREAT, 0755);
  if (descriptor == -1) {
    std::perror("Couldn't create mount point inside tmpfs");
    close(descriptor);
    return false;
  }
  close(descriptor);

  auto origin = std::format("/proc/self/fd/{}", originalFd);

  // clang-format off
  if(mount(origin.c_str(), binPath.c_str(), nullptr,MS_BIND,nullptr)!=0){
    std::perror(std::format("Couldn't mount bin env. Origin:{}, path:{}",origin,binPath.c_str()).c_str());
    return false;
  }

  if(mount(nullptr, binPath.c_str(), nullptr, MS_BIND|MS_REMOUNT|MS_RDONLY|MS_NOSUID|MS_NODEV, nullptr)!= 0){
    std::perror(std::format("Couln't remount bin env as read only: {}",binPath.c_str()).c_str());
    return false;
  }

  //clang-format on
  return true;
}

bool psx::changeWorkDir(const std::filesystem::path &workDir){
    std::error_code ec;
    std::filesystem::create_directories(workDir, ec);
    if(ec){
        std::cerr<<ec.message();
        return false;
    }

    if(chdir(workDir.c_str()) && chdir("/")){
        std::perror(std::format("Couldn't change workdir: {}", workDir.c_str()).c_str());
        return false;
    }

    return true;
}



bool psx::mountTmpfsDirs() {
  return mountTmpfsDir("/home") && mountTmpfsDir("/root") &&
         mountTmpfsDir("/run/user");
}

bool psx::mountTmpfsDir(const char *path) {
    auto res = mount("tmpfs", path, "tmpfs", MS_NOSUID | MS_NODEV, "size=1m,mode=755");
  if (res != 0 && res != ENOENT) {
    std::perror(std::format("couldn't mount tmpfs at {}", path).c_str());
    return false;
  }
  return true;
}

bool psx::isInside(const std::filesystem::path &path,
                   const std::filesystem::path &dir) {
  auto rel = path.lexically_normal().lexically_relative(dir);
  return  !rel.empty() && *rel.begin() != "..";
}

bool psx::blockSeccompProcess(pid_t pid) {
#if defined(__x86_64__)
  user_regs_struct regs{};
  if (ptrace(PTRACE_GETREGS, pid, nullptr, &regs) == -1) {
    return false;
  }

  regs.orig_rax = static_cast<unsigned long long>(-1);
  regs.rax = static_cast<unsigned long long>(-EPERM);

  return ptrace(PTRACE_SETREGS, pid, nullptr, &regs) != -1;

#else
#error "blockSeccompProcess: unsupported architecture"
#endif
}

// Family and type of a socket() call
std::string psx::describeSocket(std::uint64_t domain, std::uint64_t type) {
  std::string_view family = "unknown family";
  switch (asFd(domain)) {
  case AF_INET:
    family = "IPv4";
    break;
  case AF_INET6:
    family = "IPv6";
    break;
  case AF_PACKET:
    family = "raw packet";
    break;
  }

  std::string_view kind = "socket";
  switch (asFd(type) & ~(SOCK_NONBLOCK | SOCK_CLOEXEC)) {
  case SOCK_STREAM:
    kind = "stream socket";
    break;
  case SOCK_DGRAM:
    kind = "datagram socket";
    break;
  case SOCK_RAW:
    kind = "raw socket";
    break;
  }
  return std::format("new {} {}", family, kind);
}

std::string psx::readStringFromProcess(pid_t pid, std::uint64_t address,
                                       std::size_t max_len) {
  constexpr std::size_t word_size = sizeof(long);

  std::size_t offset = address % word_size;
  auto addr = address - offset;

  std::string result;
  while (true) {
    errno = 0;
    long word =
        ptrace(PTRACE_PEEKDATA, pid, reinterpret_cast<void *>(addr), nullptr);
    if (errno) {
      return "";
    }

    const char *bytes = reinterpret_cast<const char *>(&word);
    for (std::size_t i = offset; i < word_size; ++i) {
      if (bytes[i] == '\0') {
        return result;
      }
      if (result.size() == max_len) {
        return "";
      }
      result += bytes[i];
    }
    offset = 0;
    addr += word_size;
  }
}

std::string psx::readProcLink(pid_t pid, const std::string &entry) {
  std::array<char, PATH_MAX> buffer{};
  auto link = std::format("/proc/{}/{}", pid, entry);
  auto len = readlink(link.c_str(), buffer.data(), buffer.size());
  if (len <= 0) {
    return "";
  }
  return std::string(buffer.data(), static_cast<std::size_t>(len));
}

std::string psx::fdTarget(pid_t pid, int fd) {
  auto target = readProcLink(pid, std::format("fd/{}", fd));
  return target.empty() ? std::format("fd {}", fd) : target;
}

std::string psx::resolveAt(pid_t pid, int dirfd, const std::string &path) {
  if (path.empty()) {
    return dirfd == AT_FDCWD ? "unreadable path" : fdTarget(pid, dirfd);
  }
  if (path.front() == '/') {
    return std::filesystem::path(path).lexically_normal().string();
  }

  auto base = dirfd == AT_FDCWD
                  ? readProcLink(pid, "cwd")
                  : readProcLink(pid, std::format("fd/{}", dirfd));
  if (base.empty()) {
    return path;
  }
  return (std::filesystem::path(base) / path).lexically_normal().string();
}

std::size_t psx::readBytesFromProcess(pid_t pid, std::uint64_t address,
                                      void *out, std::size_t size) {
  iovec local{out, size};
  iovec remote{reinterpret_cast<void *>(address), size};
  auto read = process_vm_readv(pid, &local, 1, &remote, 1, 0);
  return read > 0 ? static_cast<std::size_t>(read) : 0;
}

std::string psx::execArgs(pid_t pid, std::uint64_t argvAddress) {
  std::string result;
  for (std::size_t i = 0; i < MAX_EXEC_ARGS; ++i) {
    std::uint64_t pointer = 0;
    auto address = argvAddress + i * sizeof(pointer);
    if (readBytesFromProcess(pid, address, &pointer, sizeof(pointer)) !=
            sizeof(pointer) ||
        pointer == 0) {
      return result;
    }
    result += ' ' + psx::readStringFromProcess(pid, pointer);
  }
  return result + " ...";
}

std::string psx::formatSockaddr(const sockaddr_storage &storage,
                                std::size_t len) {
  if (len < sizeof(sa_family_t)) {
    return "unreadable address";
  }

  switch (storage.ss_family) {
  case AF_INET: {
    const auto &in = reinterpret_cast<const sockaddr_in &>(storage);
    std::array<char, INET_ADDRSTRLEN> ip{};
    inet_ntop(AF_INET, &in.sin_addr, ip.data(), ip.size());
    return std::format("{}:{}", ip.data(), ntohs(in.sin_port));
  }
  case AF_INET6: {
    const auto &in6 = reinterpret_cast<const sockaddr_in6 &>(storage);
    std::array<char, INET6_ADDRSTRLEN> ip{};
    inet_ntop(AF_INET6, &in6.sin6_addr, ip.data(), ip.size());
    return std::format("[{}]:{}", ip.data(), ntohs(in6.sin6_port));
  }
  case AF_UNIX: {
    const auto &un = reinterpret_cast<const sockaddr_un &>(storage);
    constexpr auto pathOffset = offsetof(sockaddr_un, sun_path);
    if (len <= pathOffset) {
      return "unix socket (unnamed)";
    }
    auto pathLen = len - pathOffset;
    if (un.sun_path[0] == '\0') {
      return "unix socket @" + std::string(un.sun_path + 1, pathLen - 1);
    }
    return "unix socket " +
           std::string(un.sun_path, strnlen(un.sun_path, pathLen));
  }
  default:
    return std::format("address family {}", storage.ss_family);
  }
}
