#include "error.h"
#include "macros.h"
#include "path.h"

int init(int argc, UNUSED const char* argv) {
  if (argc != 0) {
    die("Usage: tig init");
  }

  create_dir(".tig");
  create_dir(".tig/objects");
  create_dir(".tig/refs");
  write_to_file(".tig/HEAD", "ref: refs/heads/master\n");

  return 0;
}
