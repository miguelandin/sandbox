// Looks for file descriptors inherited from sandbox (e.g. the -o log file).
// A sandboxed binary must only inherit stdin, stdout and stderr.
// Expected: CONTAINED always; ESCAPED lists the leaked descriptors.
#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <string>
#include <unistd.h>

int main() {
  DIR *dir = opendir("/proc/self/fd");
  if (dir == nullptr) {
    std::puts("UNKNOWN cannot open /proc/self/fd");
    return 0;
  }

  int own = dirfd(dir); // the descriptor used for this very listing
  std::string leaked;

  while (dirent *entry = readdir(dir)) {
    if (entry->d_name[0] == '.') {
      continue;
    }
    int fd = std::atoi(entry->d_name);
    if (fd > 2 && fd != own) {
      char target[512] = {};
      std::string link = "/proc/self/fd/" + std::to_string(fd);
      if (readlink(link.c_str(), target, sizeof(target) - 1) == -1) {
        target[0] = '?';
      }
      leaked += " " + std::to_string(fd) + "->" + target;
    }
  }
  closedir(dir);

  if (leaked.empty()) {
    std::puts("CONTAINED");
  } else {
    std::printf("ESCAPED%s\n", leaked.c_str());
  }
  return 0;
}
