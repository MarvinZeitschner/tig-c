#include "strbuf.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"

/**
 * Used as default value so that the string is NUL terminated even for
 * initialized strbuf
 */
char strbuf_slopbuf[1];

void strbuf_init(struct strbuf *sb, size_t hint) {
  struct strbuf blank = STRBUF_INIT;
  memcpy(sb, &blank, sizeof(*sb));
  if (hint) {
    strbuf_grow(sb, hint);
  }
}

void strbuf_grow(struct strbuf *sb, size_t extra) {
  int new_buf = !sb->alloc;

  if (new_buf) {
    sb->buf = NULL;
  }
  ALLOC_GROW(sb->buf, sb->len + extra + 1, sb->alloc);
  if (new_buf) {
    sb->buf[0] = '\0';
  }
}

void strbuf_addstr(struct strbuf *sb, char *str, size_t size) {
  strbuf_grow(sb, size);
  memcpy(sb->buf + sb->len, str, size);
  sb->len = sb->len + size;
}

void strbuf_release(struct strbuf *sb) {
  if (sb->alloc) {
    free(sb->buf);
    strbuf_init(sb, 0);
  }
}

void strbuf_addf(struct strbuf *sb, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  int needed = vsnprintf(NULL, 0, fmt, args);

  if (needed < 0) {
    va_end(args);
    die("vsnprintf");
  }

  strbuf_grow(sb, needed);

  vsnprintf(sb->buf + sb->len, needed + 1, fmt, args);
  sb->len += needed;

  va_end(args);
}
