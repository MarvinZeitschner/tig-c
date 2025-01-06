#include "object.h"
#include "path.h"
#include "strbuf.h"
#include "error.h"
#include "utils/compression/compression.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int read_object(struct object *object, struct strbuf *data, const char* hash) {
    struct strbuf path = STRBUF_INIT;

    hash_to_obj_path(&path, hash);

    struct strbuf raw_data = STRBUF_INIT;
    if(decompress_file(&raw_data, path.buf) == -1) {
        strbuf_release(&path);
        return -1;
    }
    strncpy(object->sha1, hash, SHA_SIZE - 1);
    object->sha1[SHA_SIZE - 1] = '\0';
    strbuf_release(&path);

    struct strbuf metadata = STRBUF_INIT;
    strbuf_addstr(&metadata, raw_data.buf, sizeof(raw_data.buf));

    strbuf_addstr(data, raw_data.buf + metadata.len, strlen(raw_data.buf + metadata.len) + 1);

    strtok(metadata.buf, " ");

    if(!strcmp(metadata.buf, "blob")) {
        object->type = BLOB;
    } else {
        object->type = TREE;
    }

    strbuf_release(&path);
    strbuf_release(&metadata);
    strbuf_release(&raw_data);

    return 0;
}
