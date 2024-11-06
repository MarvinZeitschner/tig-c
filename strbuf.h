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

/**
 * strbuf_init - Initialize an NUL terminated, dyanmic string buffer
 *
 * @sb: The string buffer to be initialized
 * @hint: Optional initial size of the dynamic strbuf
 *
 */
void strbuf_init(struct strbuf *sb, size_t hint);

/**
 * strbuf_grow - Grow a given strbuf by a specified size_t
 *
 * @sb: Given strbuf to be grown
 * @extra: Extra size the strbuf should be grown by
 *
 */
void strbuf_grow(struct strbuf *sb, size_t extra);

/**
 * strbuf_addstr - add a string to the end of the strbuf
 *
 * @sb: strbuf to where the string gets added
 * @str: string to be added
 * @size: size of the item to be added
 */
void strbuf_addstr(struct strbuf *sb, char *str, size_t size);

#endif
