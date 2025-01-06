#include "error.h"
#include <getopt.h>
#include <stdio.h>

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
      printf("name only\n");
      break;
    default:
      die("Unknown option");
    }
  }

  if (optind < argc) {
    printf("Remaining argument (tree-sha): %s\n", argv[optind]);
  } else {
    die("Error: No tree SHA provided.");
  }

  return 0;
}
