#define CD_BUILTIN
#include "../shell.h"
#include "../builtins.h"
#include <stdio.h>

int builtin_cd(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "Not enough args\n");
    fprintf(stderr, "Usage: cd <dir>\n");
    return -1;
  }
  return shell_set_cwd(argv[1]);
}
