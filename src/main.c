#include <stdlib.h>
#include <stdio.h>
#include "shell.h"

int main(int argc, char **argv) {
  shell_state state = {.pwd = "n/a"};
  while (1) {
    shell_expr expr;

    // prompt for input
    if (shell_prompt(&state, &expr) == -1) continue;
    shell_expr_debug(expr);

    // shell_run_cmd(expr);
  }
}
