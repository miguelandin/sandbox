// Tries to create a file outside the sandbox (/tmp is shared with the host).
// Expected: CONTAINED without -w, ESCAPED with -w.
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

int main() {
  int fd = open("/tmp/sandbox_test_marker", O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd == -1) {
    std::puts("CONTAINED");
    return 0;
  }
  dprintf(fd, "written from inside the sandbox\n");
  close(fd);
  std::puts("ESCAPED");
  return 0;
}
