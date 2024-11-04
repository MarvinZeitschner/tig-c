#include "strbuf.h"

#include <stdlib.h>
#include <string.h>

#include "alloc.h"

/**
 * Used as default value so that the string is NUL terminated even for
 * initialized strbuf
 */
char strbuf_slopbuf[1];

/**
 * strbuf_init - Initialize an NUL terminated, dyanmic string buffer
 *
 * @sb: The string buffer to be initialized
 * @hint: Optional initial size of the dynamic strbuf
 *
 */
void strbuf_init(struct strbuf *sb, size_t hint) {
  struct strbuf blank = STRBUF_INIT;
  memcpy(sb, &blank, sizeof(*sb));
  if (hint) {
    strbuf_grow(sb, hint);
  }
}

/**
 * strbuf_grow - Grow a given strbuf by a specified size_t
 *
 * @sb: Given strbuf to be grown
 * @extra: Extra size the strbuf should be grown by
 *
 */
void strbuf_grow(struct strbuf *sb, size_t extra) {
  int new_buf = !sb->alloc;
  // TODO: add unsigned_overflow check
  if (new_buf) {
    sb->buf = NULL;
  }
  ALLOC_GROW(sb->buf, sb->len + extra + 1, sb->alloc);
  if (new_buf) {
    sb->buf[0] = '\0';
  }
}
