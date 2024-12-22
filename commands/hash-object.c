#include "error.h"
#include "macros.h"
#include "object-file.h"
#include "path.h"
#include <unistd.h>

int hash_object(int argc, const char *argv[]) {
  if (argc > 2) {
    die("Usage: tig hash-object <file> [-w]");
  }
  const char *path = argv[0];

  int opt;
  enum { NONE, WRITE_MODE } mode = NONE;

  while ((opt = getopt(argc, (char *const *)argv, "w")) != -1) {
    switch (opt) {
    case 'w':
      mode = WRITE_MODE;
      break;
    default:
      die("Usage: tig hash-object <file> [-w]");
    }
  }

  if ((int)(argc - 1) != (int)optind - 1) {
    die("Usage: tig hash-object <file> [-w]");
  }

  char metadata[METADATA_MAX];
  char obj_path[PATH_MAX];
  char dir_path[PATH_MAX];
  if (hash_object_file(metadata, obj_path, dir_path, path) != 0) {
    error("Failed to create a hash from file: %s", path);
    return -1;
  }
  if (mode == WRITE_MODE) {
    compress_file_to_obj_file(metadata, dir_path, obj_path, path);
  }

  return 0;
}
