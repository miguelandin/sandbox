#include "Warden.hpp"
#include <cstdio>
#include <fcntl.h>
#include <seccomp.h>
#include <sys/socket.h>

bool sec::loadSeccomp() {
  auto ctx = createNewFilter();
  if (!ctx) {
    std::perror("seccomp failed");
    return false;
  }
  setRules(ctx);
  return true;
}

scmp_filter_ctx sec::createNewFilter() { return seccomp_init(SCMP_ACT_ALLOW); }

void sec::setRules(scmp_filter_ctx ctx) {
  setDefaultRule(ctx);
  setNetworkRule(ctx);
  setWriteRule(ctx);
  setReadRule(ctx);
  setReadWriteRule(ctx);
  setExecRule(ctx);
}

// clang-format off
void sec::setDefaultRule(scmp_filter_ctx ctx) {
  seccomp_rule_add(ctx, t_DENY, SCMP_SYS(io_uring_setup), 0);
  seccomp_rule_add(ctx, t_DENY, SCMP_SYS(io_uring_enter), 0);
  seccomp_rule_add(ctx, t_DENY, SCMP_SYS(io_uring_register), 0);
  seccomp_rule_add(ctx, t_DENY, SCMP_SYS(open_by_handle_at), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ERRNO(ENOSYS), SCMP_SYS(openat2), 0);
}

void sec::setNetworkRule(scmp_filter_ctx ctx) {
  seccomp_rule_add(ctx, t_NETWORK, SCMP_SYS(socket), 1, SCMP_A0(SCMP_CMP_EQ, AF_INET));
  seccomp_rule_add(ctx, t_NETWORK, SCMP_SYS(socket), 1, SCMP_A0(SCMP_CMP_EQ, AF_INET6));
  seccomp_rule_add(ctx, t_NETWORK, SCMP_SYS(socket), 1, SCMP_A0(SCMP_CMP_EQ, AF_PACKET));
  seccomp_rule_add(ctx, t_NETWORK, SCMP_SYS(connect), 0);
  seccomp_rule_add(ctx, t_NETWORK, SCMP_SYS(bind), 0);
  seccomp_rule_add(ctx, t_NETWORK, SCMP_SYS(listen), 0);
  seccomp_rule_add(ctx, t_NETWORK, SCMP_SYS(accept), 0);
  seccomp_rule_add(ctx, t_NETWORK, SCMP_SYS(accept4), 0);
}

void sec::setReadRule(scmp_filter_ctx ctx) {
  seccomp_rule_add(ctx, t_READ, SCMP_SYS(open), 1, SCMP_A1(SCMP_CMP_MASKED_EQ, O_ACCMODE, O_RDONLY));
  seccomp_rule_add(ctx, t_READ, SCMP_SYS(openat), 1, SCMP_A2(SCMP_CMP_MASKED_EQ, O_ACCMODE, O_RDONLY));
  seccomp_rule_add(ctx, t_READ, SCMP_SYS(read), 1, SCMP_A0(SCMP_CMP_GT, 0));
  seccomp_rule_add(ctx, t_READ, SCMP_SYS(readv), 1, SCMP_A0(SCMP_CMP_GT, 0));
  seccomp_rule_add(ctx, t_READ, SCMP_SYS(pread64), 1, SCMP_A0(SCMP_CMP_GT, 0));
  seccomp_rule_add(ctx, t_READ, SCMP_SYS(preadv), 1, SCMP_A0(SCMP_CMP_GT, 0));
  seccomp_rule_add(ctx, t_READ, SCMP_SYS(preadv2), 1, SCMP_A0(SCMP_CMP_GT, 0));
  seccomp_rule_add(ctx, t_READ, SCMP_SYS(readlink), 0);
  seccomp_rule_add(ctx, t_READ, SCMP_SYS(readlinkat), 0);
  seccomp_rule_add(ctx, t_READ, SCMP_SYS(getdents), 0);
  seccomp_rule_add(ctx, t_READ, SCMP_SYS(getdents64), 0);
}

void sec::setWriteRule(scmp_filter_ctx ctx){
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(open),   1, SCMP_A1(SCMP_CMP_MASKED_EQ, O_ACCMODE, O_WRONLY));
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(open),   1, SCMP_A1(SCMP_CMP_MASKED_EQ, O_CREAT, O_CREAT));
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(open),   1, SCMP_A1(SCMP_CMP_MASKED_EQ, O_TRUNC, O_TRUNC));
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(openat), 1, SCMP_A2(SCMP_CMP_MASKED_EQ, O_ACCMODE, O_WRONLY));
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(openat), 1, SCMP_A2(SCMP_CMP_MASKED_EQ, O_CREAT, O_CREAT));
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(openat), 1, SCMP_A2(SCMP_CMP_MASKED_EQ, O_TRUNC, O_TRUNC));
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(write), 1, SCMP_A0(SCMP_CMP_GT, 2));
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(writev), 1, SCMP_A0(SCMP_CMP_GT, 2));
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(sendfile), 1, SCMP_A0(SCMP_CMP_GT, 2));
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(pwrite64), 1, SCMP_A0(SCMP_CMP_GT, 2));
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(pwritev), 1, SCMP_A0(SCMP_CMP_GT, 2));
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(pwritev2), 1, SCMP_A0(SCMP_CMP_GT, 2));
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(creat), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(copy_file_range), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(splice), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(fallocate), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(truncate), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(ftruncate), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(unlink), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(unlinkat), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(rmdir), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(rename), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(renameat), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(renameat2), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(chmod), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(fchmod), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(lchown), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(fchownat), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(utime), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(utimes), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(utimensat), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(futimesat), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(setxattr), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(lremovexattr), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(fremovexattr), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(mkdir), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(mkdirat), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(mknod), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(mknodat), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(link), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(linkat), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(symlink), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(symlinkat), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(fchmodat), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(fchmodat2), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(chown), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(fchown), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(lsetxattr), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(fsetxattr), 0);
    seccomp_rule_add(ctx, t_WRITE, SCMP_SYS(removexattr), 0);
}

void sec::setReadWriteRule(scmp_filter_ctx ctx){
    seccomp_rule_add(ctx, t_READ_WRITE, SCMP_SYS(open),   1, SCMP_A1(SCMP_CMP_MASKED_EQ, O_ACCMODE, O_RDWR));
    seccomp_rule_add(ctx, t_READ_WRITE, SCMP_SYS(openat), 1, SCMP_A2(SCMP_CMP_MASKED_EQ, O_ACCMODE, O_RDWR));
}

void sec::setExecRule(scmp_filter_ctx ctx){
    seccomp_rule_add(ctx, t_EXEC, SCMP_SYS(execve),   0);
    seccomp_rule_add(ctx, t_EXEC, SCMP_SYS(execveat), 0);
}
// clang-format on
