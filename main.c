#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "commands.h"
#include "object-file.h"
#include "tig.h"

int main(int argc, const char** argv) {
  for (int i = 0; i < argc; ++i) {
    printf("argv[%d]: %s\n", i, argv[i]);
  }

  int res = exec_cmd(argc, argv);

  return res;
}
