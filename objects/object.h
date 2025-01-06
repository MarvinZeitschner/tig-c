#include "strbuf.h"
#include <openssl/evp.h>

#define SHA_SIZE 41

enum obj_type {
    BLOB,
    TREE
};

struct object {
    enum obj_type type;
    char sha1[SHA_SIZE];
};

int read_object(struct object *object, struct strbuf *data, const char* hash);

int hash_file(char hash[SHA_SIZE], struct strbuf* metadata,const char *path);
