#include "macros.h"
#include "path.h"

int init(UNUSED int argc, UNUSED const char* argv) {
  create_dir(".tig");
  create_dir(".tig/objects");
  create_dir(".tig/refs");
  write_to_file(".tig/HEAD", "ref: refs/heads/master\n");

  return 0;
}
