#include "error.h"
#include "objects/object.h"
#include <getopt.h>
#include <stdio.h>

enum { DEFAULT, NAME_ONLY } mode = DEFAULT;

int ls_tree(int argc, const char **argv) {
  printf("lstree\n");
  if (argc > 3) {
    die("Usage: tig ls-tree --name-only <tree-sha>");
  }

  struct option long_options[] = {
      {"name-only", no_argument, NULL, 'n'},
      {NULL, 0, NULL, 0},
  };

  int opt;
  while ((opt = getopt_long(argc, (char *const *)argv, "n", long_options,
                            NULL)) != -1) {
    switch (opt) {
    case 'n':
      mode = NAME_ONLY;
      break;
    default:
      die("Unknown option");
    }
  }

  if (optind > argc) {
    die("Error: No tree SHA provided.");
  }

  struct object object;
  get_object(&object, argv[optind]);

  struct tree tree;
  get_tree(&object, &tree);

  return 0;
}
