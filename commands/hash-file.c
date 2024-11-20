
#include "macros.h"
#include "object-file.h"

int hash_file(UNUSED int argc, const char** argv) {
  hash_object_file(argv[2]);
  return 0;
}
