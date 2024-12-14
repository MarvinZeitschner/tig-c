#include <stdio.h>

#include "commands.h"
#include "object-file.h"
#include "tig.h"

int main(int argc, const char** argv) {
  int res = exec_cmd(argc, argv);

  return res;
}
