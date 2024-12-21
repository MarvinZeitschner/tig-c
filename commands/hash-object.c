#include "error.h"
#include "macros.h"
#include "object-file.h"

int hash_object(int argc, const char **argv) {
  if (argc != 1) {
    die("Usage: tig hash-object <file>");
  }
  compress_file_to_obj_file(argv[0]);
  return 0;
}
