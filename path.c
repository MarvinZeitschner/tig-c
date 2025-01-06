#include "strbuf.h"
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

void create_dir(const char *dir) {
  if (mkdir(dir, 0777) < 0) {
    perror(dir);
  }
}

void write_to_file(const char *filename, const char *text) {
  FILE *file_ptr = fopen(filename, "w");

  if (file_ptr == NULL) {
    perror(filename);
    return;
  }
  fprintf(file_ptr, "%s", text);

  fclose(file_ptr);
}

int hash_to_obj_path(struct strbuf *sb, const char *hash) {
  strbuf_addf(sb, ".tig/objects/%.2s/%s", hash, hash + 2);

  return 0;
}
