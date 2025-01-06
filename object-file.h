// #ifndef OBJECT_FILE_H
// #define OBJECT_FILE_H

// #include "strbuf.h"
// #include <openssl/evp.h>

// #define METADATA_MAX 256

// struct object_file {
//   struct strbuf t_path;
//   unsigned int t_size;
//   struct strbuf dir_path;
//   struct strbuf obj_path;
//   struct strbuf metadata;
//   char hash[EVP_MAX_MD_SIZE];
// };

// void object_file_init(struct object_file *of);

// int object_file_get(struct object_file *of, const char *path);

// void object_file_release(struct object_file *of);

// //__

// int get_object_path(char buffer[], const char *hash);

// int decompress_object_file(const char *hash);

// int compress_file_to_obj_file(struct object_file *of, const char *path);

// int hash_object_file(struct object_file *of, const char *path);
// #endif
