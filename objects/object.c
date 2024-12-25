#include "object.h"
#include "error.h"
#include "strbuf.h"
#include "utils/compression/compression.h"
#include <string.h>
#include <sys/stat.h>

int hash_to_obj_path(struct strbuf *sb, const char *hash) {
  strbuf_addf(sb, ".tig/objects/%.2s/%s", hash, hash + 2);

  return 0;
}

int read_from_database(struct object *obj, const char *hash) {
  struct strbuf sb = STRBUF_INIT;

  hash_to_obj_path(&sb, hash);

  struct stat st;

  if (stat(sb.buf, &st) != 0) {
    error("File doesnt exist: %s", sb.buf);
    return -1;
  }

  struct strbuf metadata = STRBUF_INIT;

  if (decompress_file_metadata(&metadata, sb.buf) != 1) {
    error("error decompressing file metadata");
    return -1;
  }

  struct object new_obj;
  if (strstr(metadata.buf, "blob")) {
    new_obj.type = BLOB;
  } else if (strstr(metadata.buf, "tree")) {
    new_obj.type = TREE;
  } else {
    error("object file has wrong format: %s", hash);
    return -1;
  }

  strncpy(new_obj.oid, (char *)hash, 40);

  memcpy(obj, &new_obj, sizeof(*obj));

  strbuf_release(&sb);
  strbuf_release(&metadata);

  return 0;
}

// int read_from_raw(struct object *obj, const char *path);
