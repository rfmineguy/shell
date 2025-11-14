#ifndef BUILTINS_H
#define BUILTINS_H

typedef enum {
  BUILTIN_NA     = -1,  // not a built in
  BUILTIN_ECHO   =  0,
  BUILTIN_EXIT,
  BUILTIN_LAST,
} builtin_echo_e;

int builtin_echo(int argc, char** argv);
int builtin_exit(int argc, char** argv);

#endif
