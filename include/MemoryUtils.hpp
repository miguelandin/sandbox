#pragma once
#include <linux/limits.h>
#include <string>

namespace mem {
inline std::string readStringFromProcess(pid_t pid, unsigned long addres,
                                         std::size_t max_len = PATH_MAX);
}
