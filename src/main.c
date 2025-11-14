#include <stdlib.h>
#include <stdio.h>
#include "shell.h"

int main(int argc, char **argv) {
  shell_init();
  while (1) {
    shell_expr expr;

    // prompt for input
    if (shell_prompt(&expr) != 0) {
      fprintf(stderr, "Error: %s at index %d\n", expr.error, expr.errloc);
      continue;
    }

    int exit_code = shell_run_expr(expr);
    (void)exit_code;
  }
}
