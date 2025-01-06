#ifndef PATH_H
#define PATH_H

#include "strbuf.h"
#define PATH_MAX 1024

void create_dir(const char *dir);

void write_to_file(const char *filename, const char *text);

int hash_to_obj_path(struct strbuf *sb, const char *hash);

#endif
