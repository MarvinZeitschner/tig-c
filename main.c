#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "path.h"

int main(int argc, char** argv) {
  if (argc <= 1) {
    printf("Welcome to tig\n");
    return 0;
  }
  // for (int i = 0; i < argc; ++i) {
  //   printf("argv[%d]: %s\n", i, argv[i]);
  // }
  if (strcmp(argv[1], "init") == 0) {
    create_dir(".tig");
    create_dir(".tig/objects");
    create_dir(".tig/refs");
    write_to_file(".tig/HEAD", "ref: refs/heads/master\n");
  }

  return 0;
}
