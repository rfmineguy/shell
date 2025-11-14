#include "shell.h"
#include "builtins.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

#define STATE_NORM 0
#define STATE_WHITESPACE 1
#define STATE_PIPE 2
#define STATE_RIGHT_ARROW 3
#define STATE_LEFT_ARROW 4
#define STATE_ID 5
#define STATE_QUOTED_STRING 6
#define STATE_END 7

shell_builtin shell_builtins[10] = {
  [BUILTIN_ECHO] = {.callback = builtin_echo, .cmd_name = "echo"},
  [BUILTIN_EXIT] = {.callback = builtin_exit, .cmd_name = "exit"},
  0
};

static inline bool iswhitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n';
}

static inline bool isidchar(char c) {
  return c == '-' || isalnum(c) || c == '.' || c == '=' || c == '/';
}

// Return
//  0 - no error
//  1 - syntax error
static int shell_tokenize(shell_state *state, shell_expr *out_expr) {
  int state_ = STATE_NORM;
  shell_cmd curr_cmd = {0};
  for (size_t i = 0; i < state->raw_input_size;) {
    if (state_ == STATE_NORM) {
      if (i == state->raw_input_size - 1) state_ = STATE_END;
      else if (iswhitespace(state->raw_input[i])) state_ = STATE_WHITESPACE;
      else if (state->raw_input[i] == '|') state_ = STATE_PIPE;
      else if (state->raw_input[i] == '>') state_ = STATE_RIGHT_ARROW;
      else if (state->raw_input[i] == '<') state_ = STATE_LEFT_ARROW;
      else if (state->raw_input[i] == '\"') state_ = STATE_QUOTED_STRING;
      else if (isidchar(state->raw_input[i])) state_ = STATE_ID;
    }
    if (state_ == STATE_END) {
      out_expr->a[out_expr->commands_size].cmd = curr_cmd;
      out_expr->a[out_expr->commands_size++].redirection_type = NONE;
      i++;
      state_ = 0;
      continue;
    }
    if (state_ == STATE_WHITESPACE) {
      i++;
      state_ = 0;
      continue;
    }
    if (state_ == STATE_PIPE) {
      out_expr->a[out_expr->commands_size].cmd = curr_cmd;
      out_expr->a[out_expr->commands_size++].redirection_type = STT_PIPE;
      curr_cmd = (shell_cmd){0};

      i++;
      state_ = 0;
      continue;
    }
    if (state_ == STATE_RIGHT_ARROW) {
      int count = 1;
      if (state->raw_input[i + 1] == '>') { count++; i++; }

      out_expr->a[out_expr->commands_size].cmd = curr_cmd;
      out_expr->a[out_expr->commands_size++].redirection_type = count == 1 ? STT_RARROW_SINGLE : STT_RARROW_DOUBLE;
      curr_cmd = (shell_cmd){0};

      i += count;
      state_ = 0;
      continue;
    }
    if (state_ == STATE_LEFT_ARROW) {
      int count = 1;
      if (state->raw_input[i + 1] == '<') { count++; i++; }

      out_expr->a[out_expr->commands_size].cmd = curr_cmd;
      out_expr->a[out_expr->commands_size++].redirection_type = count == 1 ? STT_LARROW_SINGLE : STT_LARROW_DOUBLE;
      curr_cmd = (shell_cmd){0};

      i += count;
      state_ = 0;
      continue;
    }
    if (state_ == STATE_QUOTED_STRING) {
      // skip quote
      size_t end = i + 1;

      // skip characters until we reach the end of the quote or the input
      while (state->raw_input[end] && state->raw_input[end] != '"') end++;
      if (!state->raw_input[end]) {
        fprintf(stderr, "Unmatched quote\n");
        out_expr->error = "Unmatched quote";
        out_expr->errloc = end;
        return 1;
      }
      curr_cmd.tokens[curr_cmd.tokens_size++] = (shell_token) {
        .begin = state->raw_input + i + 1, .length = end - i - 1, .type = STT_QUOTED_STRING,
      };
      state->raw_input[end] = 0;
      curr_cmd.argv[curr_cmd.tokens_size - 1] = state->raw_input + i + 1;
      i += end - i;
      state_ = 0;
      continue;
    }
    if (state_ == STATE_ID) {
      size_t end = i;
      while (isidchar(state->raw_input[end])) end++;
      curr_cmd.tokens[curr_cmd.tokens_size++] = (shell_token) {
        .begin = state->raw_input + i, .length = end - i, .type = STT_ID
      };
      state->raw_input[end] = 0;
      curr_cmd.argv[curr_cmd.tokens_size - 1] = state->raw_input + i;
      i += end - i;
      state_ = 0;
      continue;
    }
    i++;
  }
  return 0;
}

// @Note: inherited from shell_tokenize
// Return
//  0 - no error
//  1 - syntax error
int shell_prompt(shell_state *state, shell_expr *out_expr) {
  if (!out_expr) return -1;
  *out_expr = (shell_expr){0};

  printf("> ");

  // 1. wait for input from user
  if (state->raw_input) {
    memset(state->raw_input, 0, state->raw_input_size);
  }
  size_t linelen = getline(&state->raw_input, &state->raw_input_size, stdin);
  state->raw_input[linelen - 1] = 0;
  state->raw_input_size = linelen;
  return shell_tokenize(state, out_expr);
}

static int shell_run_builtin(shell_cmd cmd, int index) {
  if (!shell_builtins[index].callback) return 0;
  return shell_builtins[index].callback(cmd.tokens_size, cmd.argv);
}

static int shell_run_non_builtin(shell_cmd cmd) {
  int pid = fork();
  if (pid == 0) {
    // child
    if (execvp(cmd.argv[0], cmd.argv) == -1) {
      perror("rfsh");
    }
  }
  else {
    int child_exit_code;
    int wait_rv = wait(&child_exit_code);
    return child_exit_code;
  }
  return 1;
}

int shell_run_expr(shell_expr expr) {
  if (expr.commands_size == 0) return 0;

  // only run the first command to start out
  shell_cmd cmd = expr.a[0].cmd;
  if (cmd.tokens_size == 0) return 0;

  int cmd_idx = 0;
  if ((cmd_idx = shell_is_cmd_builtin(cmd)) != BUILTIN_NA)
    return shell_run_builtin(cmd, cmd_idx);
  return shell_run_non_builtin(cmd);
}

int shell_is_cmd_builtin(shell_cmd cmd) {
  for (int i = 0; i < BUILTIN_LAST; i++) {
    if (strcmp(cmd.tokens[0].begin, shell_builtins[i].cmd_name) == 0) return i;
  }
  return BUILTIN_NA;
}

void shell_expr_debug(shell_expr expr) {
  printf(" || DEBUG SHELL EXPR ||\n");
  for (int i = 0; i < expr.commands_size; i++) {
    printf("%d: [", i);
    shell_cmd cmd = expr.a[i].cmd;
    for (int i = 0; i < cmd.tokens_size; i++) {
      printf("%.*s", (int)cmd.tokens[i].length, cmd.tokens[i].begin);
      if (i < cmd.tokens_size - 1) printf(", ");
    }
    printf("]\n");
  }
}

void shell_expr_reconstruct(shell_expr expr) {
  printf(" || RECONSTRUCT SHELL EXPR ||\n");
  for (int i = 0; i < expr.commands_size; i++) {
    shell_cmd cmd = expr.a[i].cmd;
    for (int i = 0; i < cmd.tokens_size; i++) {
      printf("%.*s ", (int)cmd.tokens[i].length, cmd.tokens[i].begin);
    }
    switch (expr.a[i].redirection_type) {
      case NONE: printf("NUL"); break;
      case STT_LARROW_SINGLE: printf(" < "); break;
      case STT_RARROW_SINGLE: printf(" > "); break;
      case STT_LARROW_DOUBLE: printf(" << "); break;
      case STT_RARROW_DOUBLE: printf(" >> "); break;
      case STT_PIPE: printf(" | "); break;
      case STT_QUOTED_STRING: assert(0 && "STT_QUOTED_STRING not a redirection type");
      case STT_ID: assert(0 && "STT_ID not a redirection type");
    }
  }
  printf("\n");
}

int shell_exit() {
  exit(0);
}
