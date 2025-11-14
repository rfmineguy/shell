#include "shell.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static inline bool iswhitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n';
}

static inline bool isidchar(char c) {
  return c == '-' || isalnum(c) || c == '.' || c == '=';
}
