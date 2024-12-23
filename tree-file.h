#ifndef TREE_FILE_H
#define TREE_FILE_H

#include "strbuf.h"

enum file_mode { regular = 100644, executable = 100755, symbolik = 120000 };

struct tree_file {
  unsigned int tree_size;
  enum file_mode mode;
  struct strbuf name;
  struct strbuf hash;
};

#endif
