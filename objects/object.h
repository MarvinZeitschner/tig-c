#include "strbuf.h"
#include <openssl/evp.h>

#define SHA_SIZE 41

enum obj_type { BLOB, TREE };

struct object {
  enum obj_type type;
  char sha1[SHA_SIZE];
  unsigned int size;
};

enum type {
  REGULAR = 100644,
  EXECUTABLE = 100755,
  SYMLINK = 120000,
};

struct tree {
  struct object object;
  struct tree_entry *entry;
};

struct tree_entry {
  enum type type;
  enum obj_type obj_type;
  struct strbuf *name;
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

char *obj_type_to_str(struct object *object);

int get_tree(struct object *object, struct tree *tree);

int init_tree_entry(struct tree_entry *entry, char *mode,
                    enum obj_type obj_type, char *name, char hash[SHA_SIZE]);
