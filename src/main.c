#include <stdlib.h>
#include <stdio.h>
#include "shell.h"

int main(int argc, char **argv) {
  shell_state state = {.pwd = "n/a"};
  while (1) {
    shell_cmd cmd;

    // prompt for input
    if (shell_prompt(&state, &cmd) == -1) continue;

    printf("tokens_size: %d\n", cmd.tokens_size);
    for (int i = 0; i < cmd.tokens_size; i++) {
      printf("%d : '%.*s'\n", cmd.tokens[i].type, (int)cmd.tokens[i].length, cmd.tokens[i].begin);
    }

    if (shell_is_cmd_builtin(cmd)) {
      printf("builtin\n");
    }
    else{
    }
  }
}
