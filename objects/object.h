#ifndef OBJECT_H
#define OBJECT_H

#include "strbuf.h"
#include <openssl/evp.h>

#define MAX_OID_SIZE 40

enum object_type {
  BLOB,
  TREE,
};

struct object {
  enum object_type type;
  char oid[MAX_OID_SIZE];
};

int hash_to_obj_path(struct strbuf *sb, const char *hash);

int read_from_database(struct object *obj, const char *hash);

int read_from_raw(struct object *obj, const char *path);

#endif
