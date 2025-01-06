#include "object.h"
#include "error.h"
#include "path.h"
#include "strbuf.h"
#include "utils/compression/compression.h"
#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int read_object(struct object *object, struct strbuf *data, const char *hash) {
  struct strbuf path = STRBUF_INIT;

  hash_to_obj_path(&path, hash);

  struct strbuf raw_data = STRBUF_INIT;
  if (decompress_file(&raw_data, path.buf) == -1) {
    strbuf_release(&path);
    return -1;
  }
  strncpy(object->sha1, hash, SHA_SIZE - 1);
  object->sha1[SHA_SIZE - 1] = '\0';
  strbuf_release(&path);

  struct strbuf metadata = STRBUF_INIT;
  strbuf_addstr(&metadata, raw_data.buf, sizeof(raw_data.buf));

  strbuf_addstr(data, raw_data.buf + metadata.len,
                strlen(raw_data.buf + metadata.len) + 1);

  strtok(metadata.buf, " ");

  if (!strcmp(metadata.buf, "blob")) {
    object->type = BLOB;
  } else {
    object->type = TREE;
  }

  strbuf_release(&path);
  strbuf_release(&metadata);
  strbuf_release(&raw_data);

  return 0;
}

int hash_file(char hash[SHA_SIZE], struct strbuf* metadata,const char *path) {
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
