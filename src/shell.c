#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define STATE_NORM 0
#define STATE_WHITESPACE 1
#define STATE_PIPE 2
#define STATE_RIGHT_ARROW 3
#define STATE_LEFT_ARROW 4
#define STATE_ID 5
#define STATE_END 6

static inline bool iswhitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n';
}

static inline bool isidchar(char c) {
  return c == '-' || isalnum(c) || c == '.' || c == '=';
}

static void shell_tokenize(shell_state *state, shell_expr *out_expr) {
  int state_ = STATE_NORM;
  shell_cmd curr_cmd = {0};
  for (size_t i = 0; i < state->raw_input_size;) {
    if (state_ == STATE_NORM) {
      if (i == state->raw_input_size - 1) state_ = STATE_END;
      else if (iswhitespace(state->raw_input[i])) state_ = STATE_WHITESPACE;
      else if (state->raw_input[i] == '|') state_ = STATE_PIPE;
      else if (state->raw_input[i] == '>') state_ = STATE_RIGHT_ARROW;
      else if (state->raw_input[i] == '<') state_ = STATE_LEFT_ARROW;
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
}

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
  shell_tokenize(state, out_expr);
  return 0;
}

static int shell_run_builtin(shell_cmd cmd, int index) {
  return shell_builtins[index](cmd.tokens_size, cmd.argv);
}

static int shell_run_non_builtin(shell_cmd cmd) {
  return 1;
}

int shell_run_cmd(shell_cmd cmd) {
  int cmd_idx = 0;
  if ((cmd_idx = shell_is_cmd_builtin(cmd)) != BUILTIN_NOT)
    return shell_run_builtin(cmd, cmd_idx);
  return shell_run_non_builtin(cmd);
}

int shell_is_cmd_builtin(shell_cmd cmd) {
  if (strncmp(cmd.tokens[0].begin, "echo", cmd.tokens[0].length) == 0) {
    return BUILTIN_ECHO_IDX;
  }
  return BUILTIN_NOT;
}

void shell_expr_debug(shell_expr expr) {
  printf("commands_size: %d\n", expr.commands_size);
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
      case STT_ID: assert(0 && "Not a redirection type");
    }
  }
  printf("\n");
}
