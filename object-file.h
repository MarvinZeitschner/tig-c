#ifndef OBJECT_FILE_H
#define OBJECT_FILE_H

#include "stdlib.h"

#define CHUNK 16384
#define METADATA_MAX 256

void get_object_path(char buffer[], const char *hash);

void decompress_object_file(const char *hash);

void hash_object_file(const char *path);

int compress_file(const char *path, const char *path_to_write);

int compress_buf(const char *buf, size_t size, const char *path_to_write);

#endif
