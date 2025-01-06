#include "error.h"
#include "path.h"
#include "macros.h"
#include "objects/object.h"
#include "strbuf.h"
#include "utils/compression/compression.h"
#include <stdio.h>
#include <unistd.h>
#include <openssl/evp.h>

int hash_object(int argc, const char *argv[]) {
  if (argc > 2) {
    die("Usage: tig hash-object <file> [-w]");
  }
  const char *path = argv[0];

  int opt;
  enum { NONE, WRITE_MODE } mode = NONE;

  while ((opt = getopt(argc, (char *const *)argv, "w")) != -1) {
    switch (opt) {
    case 'w':
      mode = WRITE_MODE;
      break;
    default:
      die("Usage: tig hash-object <file> [-w]");
    }
  }

  if ((int)(argc - 1) != (int)optind - 1) {
    die("Usage: tig hash-object <file> [-w]");
  }

  char hash[EVP_MAX_MD_SIZE];
  struct strbuf metadata;
  if(hash_file(hash, &metadata, path) == -1) {
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
