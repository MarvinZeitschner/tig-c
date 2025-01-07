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
  get_object(&object, argv[1]);
  struct strbuf data = STRBUF_INIT;

  if (read_object(&object, &data) != 0) {
    return -1;
  }

  fwrite(data.buf, 1, data.len - 1, stdout);
  strbuf_release(&data);
  // TODO: release object
  return 0;
}
