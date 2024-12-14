#include "error.h"
#include "macros.h"
#include "object-file.h"

int cat_file(int argc, const char** argv) {
  if (argc != 1) {
    die("Usage: tig cat-file <hash>\n");
  }
  decompress_object_file(argv[0]);
  return 0;
}
