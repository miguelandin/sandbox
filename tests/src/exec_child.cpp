// Tries to execute another binary after being launched.
// Expected: CONTAINED without -x, ESCAPED with -x (printed by echo itself).
#include <cstdio>
#include <unistd.h>

int main() {
  char *const args[] = {const_cast<char *>("echo"),
                        const_cast<char *>("ESCAPED"), nullptr};
  execv("/usr/bin/echo", args);

  // Only reached if execv failed
  std::puts("CONTAINED");
  return 0;
}
