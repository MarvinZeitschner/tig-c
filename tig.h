#ifndef TIG_H
#define TIG_H

#include "commands.h"

struct cmd_struct {
  char *name;
  int (*fn)(int argc, const char **argv);
};

static struct cmd_struct command_list[] = {
    {"init", init},
    {"cat-file", cat_file},
    {"hash-object", hash_object},
    {"ls-tree", ls_tree},
};

int exec_cmd(int argc, const char **argv);

#endif
