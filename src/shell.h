#ifndef SHELL_H
#define SHELL_H
#define MAX_TOKENS 100

#include <stdbool.h>
#include <stddef.h>

typedef enum {
  STT_ID, STT_PIPE,
  STT_LARROW_SINGLE, STT_LARROW_DOUBLE,
  STT_RARROW_SINGLE, STT_RARROW_DOUBLE,
} shell_token_type;

typedef struct {
  char* begin;
  size_t length;
  int type;
} shell_token;

typedef struct {
  char* pwd;
  char* raw_input;
  size_t raw_input_size;
} shell_state;

typedef struct {
  const char* raw;
  int raw_length;

  shell_token tokens[MAX_TOKENS];
  int tokens_size;
} shell_cmd;

/*
 * @Desc:
 *    Prompts the user for input
 * @Param:
 *    state: valid pointer to a state struct
 * @Return:
 *    0  - if there is input to parse
 *    -1 - if there is no input to parse
 */
int shell_prompt(shell_state* state, shell_cmd* out_cmd);

/*
 * @Desc:
 *    Determines whether a command is an internal or external command
 * @Param:
 *    state: valid pointer to a state struct
 * @Return:
 *    t/f depending on whether its builtin or not
 */
bool shell_is_cmd_builtin(shell_cmd cmd);

#endif
