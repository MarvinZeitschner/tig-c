#ifndef OBJECT_FILE_H
#define OBJECT_FILE_H

#define CHUNK 16384

void get_object_path(char buffer[], const char* hash);

void decompress_object_file(const char* hash);

void hash_object_file(const char* path);

#endif
