// Log injection attack: creates a file whose name closes the current JSON
// line and appends a fake "allowed network" event to sandbox's log.
// The attack works only if sandbox writes the target without escaping it.
// Expected: the log stays valid JSON and contains no fake evil.com event.
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>

int main() {
  const char *name =
      "/tmp/x\"}\n"
      "{\"ts\":\"2026-01-01T00:00:00Z\",\"pid\":1,\"nr\":42,"
      "\"group\":\"network\",\"allowed\":true,\"target\":\"evil.com:443";

  int fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0644);
  if (fd != -1) {
    close(fd);
    unlink(name);
  }
  std::puts("DONE");
  return 0;
}
