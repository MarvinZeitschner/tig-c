#include "strbuf.h"
#include <openssl/evp.h>

#define SHA_SIZE 41

enum obj_type { BLOB, TREE };

struct object {
  enum obj_type type;
  char sha1[SHA_SIZE];
};

enum type {
  regular = 100644,
  executable = 100755,
  symbolik = 120000,
};

struct tree {
  struct strbuf name;
  char sha1[SHA_SIZE];
  struct object object;
  struct tree_entry **entry;
};

struct tree_entry {
  enum type type;
  struct strbuf name;
  char sha1[SHA_SIZE];
};

/**
 * get_object - initialize an object from a given hash by reading file metadata
 * from dis
 *
 * @object: The object pointer to be mutated with the object information
 * @hash: The hash for the file to be read from the object database
 *
 */
int get_object(struct object *object, const char *hash);

/**
 * read_object - Given an object read and return a pointer to the decompressed
 * object content
 *
 * @object: The object pointer from which should be read
 * @data: The raw data from the decompressed object file
 *
 */
int read_object(struct object *object, struct strbuf *data);

/**
 * hash_file - Generate a hash from a given file with the possibility to append
 * metadata
 *
 * @hash: The hash that will be generated
 * @metadata: The metadata that will be prepended and seperated with a null
 * terminator
 * @path: The path to the file
 *
 */
int hash_file(char hash[SHA_SIZE], struct strbuf *metadata, const char *path);
