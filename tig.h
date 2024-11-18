#ifndef TIG_H
#define TIG_H

struct cmd_struct {
  char* name;
  int (*fn)(int, const char**);
};

int exec_cmd(int argc, const char** argv);

#endif
