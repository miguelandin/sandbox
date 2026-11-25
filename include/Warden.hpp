#pragma once
#include <cerrno>
#include <cstdint>
#include <seccomp.h>

namespace sec {
enum class Group : std::uint16_t { NETWORK = 1, READ = 2, WRITE = 4, EXEC = 8 };

constexpr inline std::uint16_t id(Group g) {
  return static_cast<std::uint16_t>(g);
}

// clang-format off
const std::uint32_t t_NETWORK = SCMP_ACT_TRACE(id(Group::NETWORK));
const std::uint32_t t_READ = SCMP_ACT_TRACE(id(Group::READ));
const std::uint32_t t_WRITE = SCMP_ACT_TRACE(id(Group::WRITE));
const std::uint32_t t_READ_WRITE = SCMP_ACT_TRACE(id(Group::READ) | id(Group::WRITE));
const std::uint32_t t_EXEC = SCMP_ACT_TRACE(id(Group::EXEC));
const std::uint32_t t_DENY = SCMP_ACT_ERRNO(EPERM);
// clang-format on

// creates and loads a new filter with initilized rules, (use it in child
// process)
bool loadSeccomp();

// create a new filter with SCMP_ACT_ALLOW (doesn't block by default)
scmp_filter_ctx createNewFilter();

// set all rules
void setRules(scmp_filter_ctx ctx);

// rules preventing evading sandbox
void setDefaultRule(scmp_filter_ctx ctx);

void setNetworkRule(scmp_filter_ctx ctx);

void setReadRule(scmp_filter_ctx ctx);

void setWriteRule(scmp_filter_ctx ctx);

void setExecRule(scmp_filter_ctx ctx);

void setReadWriteRule(scmp_filter_ctx ctx);

} // namespace sec
