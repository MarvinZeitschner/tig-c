#include "object.h"
#include "error.h"
#include "path.h"
#include "strbuf.h"
#include "z_compression.h"
#include <openssl/evp.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int get_object(struct object *object, const char *hash) {
  struct strbuf path = STRBUF_INIT;

  hash_to_obj_path(&path, hash);

  struct strbuf metadata = STRBUF_INIT;
  if (decompress_file_metadata(&metadata, path.buf) == -1) {
    strbuf_release(&path);
    strbuf_release(&metadata);
    return -1;
  }

  // TODO: write strbuf_strtok() as strtok modifies the strbuf
  char *token = strtok(metadata.buf, " ");
  char *size_str = strtok(NULL, "\0");

  strncpy(object->sha1, hash, SHA_SIZE - 1);
  object->sha1[SHA_SIZE - 1] = '\0';
  if (!strcmp(token, "blob")) {
    object->type = BLOB;
  } else {
    object->type = TREE;
  }
  char *endptr;
  long num = strtol(size_str, &endptr, 10);
  if (endptr == size_str) {
    error("No digits were found.\n");
    strbuf_release(&path);
    strbuf_release(&metadata);
    return -1;
  } else if (*endptr != '\0') {
    strbuf_release(&path);
    strbuf_release(&metadata);
    return -1;
    error("Invalid character: %c\n", *endptr);
  }
  object->size = num;

  strbuf_release(&path);
  strbuf_release(&metadata);

  return 0;
}

int read_object(struct object *object, struct strbuf *data) {
  struct strbuf path = STRBUF_INIT;

  hash_to_obj_path(&path, object->sha1);

  struct strbuf raw_data = STRBUF_INIT;
  if (decompress_file(&raw_data, path.buf) == -1) {
    strbuf_release(&path);
    return -1;
  }

  char *content = strchr(raw_data.buf, '\0') + 1;
  strbuf_addstr(data, content, strlen(content));

  strbuf_release(&raw_data);
  strbuf_release(&path);

  return 0;
}

int hash_file(char hash[SHA_SIZE], struct strbuf *metadata, const char *path) {
  struct stat st;

  if (stat(path, &st) != 0) {
    perror("");
    return -1;
  }

  FILE *file = fopen(path, "rb");
  if (!file) {
    return -1;
  }

  unsigned char hash_buf[EVP_MAX_MD_SIZE];
  unsigned char in_buffer[CHUNK];
  size_t bytes_read = 0;
  EVP_MD_CTX *hashctx = EVP_MD_CTX_new();

  EVP_MD_CTX_init(hashctx);
  EVP_DigestInit_ex(hashctx, EVP_sha1(), NULL);

  strbuf_addf(metadata, "blob %d", st.st_size);
  EVP_DigestUpdate(hashctx, metadata->buf, metadata->len + 1);

  do {
    bytes_read = fread(in_buffer, 1, CHUNK, file);
    if (!bytes_read) {
      break;
    }

    EVP_DigestUpdate(hashctx, in_buffer, bytes_read);
  } while (bytes_read == CHUNK);
  fclose(file);

  unsigned int outlen;
  EVP_DigestFinal_ex(hashctx, hash_buf, &outlen);
  EVP_MD_CTX_free(hashctx);

  for (unsigned int i = 0; i < outlen; i++) {
    snprintf(&hash[i * 2], 3, "%02x", hash_buf[i]);
  }

  return 0;
}

int get_tree(struct object *object, struct tree *tree) {
  struct strbuf path = STRBUF_INIT;

  hash_to_obj_path(&path, object->sha1);

  struct strbuf raw_data = STRBUF_INIT;

  decompress_file(&raw_data, path.buf);

  char *content = strchr(raw_data.buf, '\0') + 1;
  char *end = content + object->size;

  while (content < end) {
    char *mode = strtok(content, " ");
    content = content + strlen(mode) + 1;
    char *name = strtok(content, "\0");
    content = content + strlen(name) + 1;
    char *raw_hash = strtok(content, " ");

    char hash[SHA_SIZE];

    for (int i = 0; i < 20; i++) {
      snprintf(&hash[i * 2], 3, "%02x", (unsigned char)raw_hash[i]);
    }

    struct object object;
    get_object(&object, hash);

    printf("%s %u %s %s\n", mode, object.type, hash, name);

    content = content + 20;
  }

  strbuf_release(&raw_data);

  return 0;
}
