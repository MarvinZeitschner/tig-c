#include "commands.h"
#include "tig.h"

int main(int argc, const char **argv) {
  int res = exec_cmd(argc, argv);

  return res;
}
