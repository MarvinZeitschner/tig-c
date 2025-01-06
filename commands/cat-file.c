#include "error.h"
#include "macros.h"
#include "objects/object.h"
#include "strbuf.h"
#include <stdio.h>

int cat_file(int argc, const char **argv) {
  if (argc != 2) {
    die("Usage: tig cat-file <hash>\n");
  }
  struct object object;
  struct strbuf sb = STRBUF_INIT;
  if (read_object(&object, &sb, argv[1]) != 0) {
    return -1;
  }

  fwrite(sb.buf, 1, sb.len - 1, stdout);
  strbuf_release(&sb);
  return 0;
}
