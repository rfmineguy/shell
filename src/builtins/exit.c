#include "../builtins.h"
#define EXIT_BUILTIN
#include "../shell.h"

int builtin_exit(int argc, char** argv) {
  return shell_exit();
}
