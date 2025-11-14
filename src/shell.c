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

static inline bool iswhitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n';
}

static inline bool isidchar(char c) {
  return c == '-' || isalnum(c) || c == '.' || c == '=';
}

static void shell_tokenize(shell_state *state, shell_cmd *out_cmd) {
  int state_ = STATE_NORM;
  for (size_t i = 0; i < state->raw_input_size;) {
    if (state_ == STATE_NORM) {
      if (iswhitespace(state->raw_input[i])) state_ = STATE_WHITESPACE;
      else if (state->raw_input[i] == '|') state_ = STATE_PIPE;
      else if (state->raw_input[i] == '>') state_ = STATE_RIGHT_ARROW;
      else if (state->raw_input[i] == '<') state_ = STATE_LEFT_ARROW;
      else if (isidchar(state->raw_input[i])) state_ = STATE_ID;
    }
    if (state_ == STATE_WHITESPACE) {
      i++;
      state_ = 0;
      continue;
    }
    if (state_ == STATE_PIPE) {
      out_cmd->tokens[out_cmd->tokens_size++] = (shell_token) {
        .begin = state->raw_input + i, .length = 1, .type = STT_PIPE
      };
      i++;
      state_ = 0;
      continue;
    }
    if (state_ == STATE_RIGHT_ARROW) {
      int count = 1;
      if (state->raw_input[i + 1] == '>') { count++; i++; }

      out_cmd->tokens[out_cmd->tokens_size++] = (shell_token) {
        .begin = state->raw_input + i, .length = count, .type = count == 1 ? STT_RARROW_SINGLE : STT_RARROW_DOUBLE
      };

      i += count;
      state_ = 0;
      continue;
    }
    if (state_ == STATE_LEFT_ARROW) {
      int count = 1;
      if (state->raw_input[i + 1] == '<') { count++; i++; }

      out_cmd->tokens[out_cmd->tokens_size++] = (shell_token) {
        .begin = state->raw_input + i, .length = count, .type = count == 1 ? STT_LARROW_SINGLE : STT_LARROW_DOUBLE
      };

      i += count;
      state_ = 0;
      continue;
    }
    if (state_ == STATE_ID) {
      size_t end = i;
      while (isidchar(state->raw_input[end])) end++;
      out_cmd->tokens[out_cmd->tokens_size++] = (shell_token) {
        .begin = state->raw_input + i, .length = end - i, .type = STT_ID
      };
      i += end - i;
      state_ = 0;
      continue;
    }
    i++;
  }
}
