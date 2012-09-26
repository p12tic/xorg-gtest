#include <signal.h>
#include <unistd.h>

int main(void) {

  signal(SIGTERM, SIG_IGN);
  sleep(100);

  return 0;
}
