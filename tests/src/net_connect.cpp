// Tries to open a TCP connection to a public host (1.1.1.1:80).
// Expected: CONTAINED without -n, ESCAPED with -n (requires internet access).
#include <arpa/inet.h>
#include <cstdio>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

int main() {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    std::puts("CONTAINED");
    return 0;
  }

  // On Linux SO_SNDTIMEO also bounds connect(), so the test never hangs
  timeval timeout{3, 0};
  setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_port = htons(80);
  inet_pton(AF_INET, "1.1.1.1", &addr.sin_addr);

  int res = connect(sock, reinterpret_cast<sockaddr *>(&addr), sizeof(addr));
  close(sock);

  std::puts(res == 0 ? "ESCAPED" : "CONTAINED");
  return 0;
}
