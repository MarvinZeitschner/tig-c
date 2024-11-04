#ifndef STRBUF_H
#define STRBUF_H

#include <stdlib.h>

extern char strbuf_slopbuf[];
#define STRBUF_INIT {.buf = strbuf_slopbuf}

struct strbuf {
  size_t alloc;
  size_t len;
  char *buf;
};

void strbuf_init(struct strbuf *sb, size_t hint);
void strbuf_grow(struct strbuf *sb, size_t extra);

#endif
