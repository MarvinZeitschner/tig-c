#include "path.h"

int init(int argc, const char* argv) {
  create_dir(".tig");
  create_dir(".tig/objects");
  create_dir(".tig/refs");
  write_to_file(".tig/HEAD", "ref: refs/heads/master\n");
}