#ifndef TREE_H
#define TREE_H

#include "object.h"

struct tree {
  struct object object;
  struct tree_entry *entries;
};

enum file_mode { regular = 100644, executable = 100755, symbolik = 120000 };

struct tree_entry {
  enum file_mode mode;
  const char *path;
  struct object *object;
};

#endif
