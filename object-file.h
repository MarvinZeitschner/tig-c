#ifndef OBJECT_FILE_H
#define OBJECT_FILE_H

#define CHUNK 16384
#define METADATA_MAX 256

void get_object_path(char buffer[], const char *hash);

void decompress_object_file(const char *hash);

int compress_file_to_obj_file(const char *path);

int hash_object_file(char *metadata, char *dir_path, char *obj_path,
                     const char *path);
#endif
