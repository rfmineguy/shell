# Shell
This project is an attempt at writing a custom shell from scratch in C. It is inspired by a school assignment given in a Linux course.

# Features
- [ ] Shell prompt with current directory listed
- [x] Command parsing
  - [x] Redirect operators (>, >>, <, <<, |)
  - [x] Quoted strings
  - [x] Paths
- [ ] Command execution
  - [x] Process fork, exec, and wait
  - [x] Argument forwarding
  - [x] Support for builtins
  - [ ] Redirect operators

# Unplanned Features
- environment variable expansion
- subshells

# Builtins
- [x] exit
- [x] echo
- [x] cd
- [ ] type
