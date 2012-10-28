#include <string.h>
#include <stdlib.h>

/**
 * Test helper. Exists with failure if "-fail yes" is passed
 */
int main(int argc, char **argv) {

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-fail") == 0)
      exit(1);

    /* test passes "-fail yes" and we expect both to be removed */
    if (strcmp(argv[i], "yes") == 0)
      exit(1);
  }

  return 0;
}
