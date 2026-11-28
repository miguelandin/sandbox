// Tries to steal a secret file from the user's home directory and print it.
// The path is passed as argv[1] by the runner.
// Expected: CONTAINED without -r (tmpfs hides /home), ESCAPED + secret with -r.
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::puts("usage: read_home <secret file>");
    return 2;
  }

  int fd = open(argv[1], O_RDONLY);
  if (fd == -1) {
    std::puts("CONTAINED");
    return 0;
  }

  char buf[256] = {};
  ssize_t n = read(fd, buf, sizeof(buf) - 1);
  close(fd);

  if (n <= 0) {
    std::puts("CONTAINED");
    return 0;
  }
  std::printf("ESCAPED %s", buf);
  return 0;
}
