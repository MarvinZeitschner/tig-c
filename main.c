#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "object-file.h"
#include "path.h"
#include "strbuf.h"

int main(int argc, char** argv) {
  if (argc <= 1) {
    printf("Welcome to tig\n");
    return 0;
  }
  for (int i = 0; i < argc; ++i) {
    printf("argv[%d]: %s\n", i, argv[i]);
  }
  if (strcmp(argv[1], "init") == 0) {
    create_dir(".tig");
    create_dir(".tig/objects");
    create_dir(".tig/refs");
    write_to_file(".tig/HEAD", "ref: refs/heads/master\n");
  } else if ((strcmp(argv[1], "cat-file") == 0) && (argc == 3)) {
    decompress_object_file(argv[2]);
  } else {
    printf("Unknown command\n");
  }

  return 0;
}
