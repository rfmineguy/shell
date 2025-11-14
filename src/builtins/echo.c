#include "../builtins.h"
#include <stdio.h>

int builtin_echo(int argc, char** argv) {
  printf("argc: %d\n", argc);
  for (int i = 0; i < argc; i++) {
    printf("%d:   %s\n", i, argv[i]);
  }
  return 0;
}
