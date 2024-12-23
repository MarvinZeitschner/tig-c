#ifndef COMPRESSION_H
#define COMPRESSION_H

#include "strbuf.h"

#define CHUNK 16384

int decompress_file(struct strbuf *sb, char *path);

int compress_to_file(char *metadata, const char *path, char *path_to_write);

#endif
