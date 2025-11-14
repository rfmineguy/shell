#ifndef SHELL_H
#define SHELL_H
#define MAX_ARGV 50
#define MAX_TOKENS 100
#define MAX_COMMAND_CHAIN 50

#include <stdbool.h>
#include <stddef.h>

typedef enum {
  NONE = 0,
  STT_ID, STT_PIPE,
  STT_LARROW_SINGLE, STT_LARROW_DOUBLE,
  STT_RARROW_SINGLE, STT_RARROW_DOUBLE,
  STT_QUOTED_STRING,
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

  char* argv[MAX_ARGV];
} shell_cmd;

typedef struct {
  struct {
    shell_cmd cmd;
    shell_token_type redirection_type;
  } a[MAX_COMMAND_CHAIN];
  int commands_size;

  // errors
  const char* error;
  int errloc;
} shell_expr;

typedef struct {
  const char* cmd_name;
  int(*callback)(int argc, char** argv);
} shell_builtin;

/*
 * @Desc:
 *    Prompts the user for input
 * @Param:
 *    state: valid pointer to a state struct
 * @Return:
 *    0  - if there is input to parse
 *    -1 - if there is no input to parse
 */
int shell_prompt(shell_state* state, shell_expr* out_expr);

/*
 * @Desc
 *    Run a command
 * @Param
 *    cmd: valid command struct given by shell_prompt
 * @Return
 *    normal : return value of the run command
 *    error  : -100
 */
int shell_run_expr(shell_expr expr);

/*
 * @Desc:
 *    Determines whether a command is an internal or external command
 * @Param:
 *    state: valid pointer to a state struct
 * @Return:
 *    t/f depending on whether its builtin or not
 */
int shell_is_cmd_builtin(shell_cmd cmd);

/*
 * @Desc:
 *    Echo the last entered expression in tokenized form
 * @Param:
 *    cmd: shell_expr struct 
 */
void shell_expr_debug(shell_expr cmd);

#ifdef EXIT_BUILTIN
int shell_exit();
#endif

#endif
