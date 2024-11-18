#ifndef TIG_H
#define TIG_H

struct cmd_struct {
  char* name;
  int (*fn)(int, const char**);
};

static struct cmd_struct* get_cmd(const char* name);
int exec_cmd(int argc, const char** argv);

#endif
