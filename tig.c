#include "tig.h"

#include <stdio.h>
#include <string.h>

#include "commands.h"
#include "error.h"
#include "macros.h"

static struct cmd_struct command_list[] = {
    {"init", init},
    {"cat-file", cat_file},
};

static struct cmd_struct* get_cmd(const char* name) {
  for (int i = 0; i < ARRAY_SIZE(command_list); i++) {
    struct cmd_struct* cmd = command_list + i;
    if (!strcmp(name, cmd->name)) {
      return cmd;
    }
  }
  return NULL;
}

int exec_cmd(int argc, const char** argv) {
  if (argc <= 1) {
    printf("Welcome to tig\n");
    return 0;
  }

  struct cmd_struct* cmd = get_cmd(argv[1]);
  if (!cmd) {
    return error("Unknown command: %s\n", argv[1]);
  }
  cmd->fn(argc, argv);

  return 0;
}
