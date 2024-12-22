#include "error.h"
#include "macros.h"
#include "object-file.h"
#include <stdio.h>
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

  struct object_file of;
  object_file_init(&of);
  if (object_file_get(&of, path) != 0) {
    object_file_release(&of);
    return -1;
  }

  // printf("\n\n %s,\n %s,\n %s,\n %s,\n %s,\n %d\n \n", of.t_path.buf,
  //        of.dir_path.buf, of.obj_path.buf, of.hash, of.metadata.buf,
  //        of.t_size);

  printf("%s\n", of.hash);

  if (mode == WRITE_MODE) {
    if (compress_file_to_obj_file(&of, path) != 0) {
      object_file_release(&of);
      return -1;
    }
  }

  object_file_release(&of);

  return 0;
}
