#include "macros.h"
#include "object-file.h"

int cat_file(UNUSED int argc, const char** argv) {
  decompress_object_file(argv[2]);
  return 0;
}
