#include "error.h"
#include "macros.h"
#include "objects/object.h"
#include "path.h"
#include "strbuf.h"
#include "utils/compression/compression.h"
#include <getopt.h>
#include <openssl/evp.h>
#include <stdio.h>

int hash_object(int argc, const char *argv[]) {
  if (argc > 3) {
    die("Usage: tig hash-object [-w] <file> ");
  }

  int opt;
  enum { NONE, WRITE_MODE } mode = NONE;

  while ((opt = getopt(argc, (char *const *)argv, "w")) != -1) {
    switch (opt) {
    case 'w':
      mode = WRITE_MODE;
      break;
    default:
      die("Usage: tig hash-object [-w] <file> ");
    }
  }

  const char *path = argv[optind];

  char hash[EVP_MAX_MD_SIZE];
  struct strbuf metadata;
  if (hash_file(hash, &metadata, path) == -1) {
    die("Error hashing file: %s", path);
  }
  printf("%s\n", hash);

  if (mode == WRITE_MODE) {
    struct strbuf path_to_write = STRBUF_INIT;
    strbuf_addf(&path_to_write, ".tig/objects/%.2s/%s", hash, hash + 2);
    struct strbuf obj_dir = STRBUF_INIT;
    strbuf_addf(&obj_dir, ".tig/objects/%.2s", hash);

    create_dir(obj_dir.buf);

    compress_to_file(metadata.buf, path, path_to_write.buf);
  }

  return 0;
}
