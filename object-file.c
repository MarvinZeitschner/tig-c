#include "object-file.h"

#include <assert.h>
#include <errno.h>
#include <openssl/evp.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <zlib.h>

#include "error.h"
#include "objects/object.h"
#include "path.h"
#include "strbuf.h"
#include "utils/compression/compression.h"

void object_file_init(struct object_file *of) {
  struct object_file blank = {0};

  strbuf_init(&blank.t_path, 0);
  strbuf_init(&blank.dir_path, 0);
  strbuf_init(&blank.obj_path, 0);
  strbuf_init(&blank.metadata, 0);

  memcpy(of, &blank, sizeof(blank));
}

int object_file_get(struct object_file *of, const char *path) {
  strbuf_addstr(&of->t_path, (char *)path, sizeof(path));

  struct stat st;

  if (stat(path, &st) != 0) {
    perror("");
    return -1;
  }

  of->t_size = st.st_size;

  strbuf_addf(&of->metadata, "blob %lld", of->t_size);

  if (hash_object_file(of, path) != 0) {
    return -1;
  }

  return 0;
}

void object_file_release(struct object_file *of) {
  strbuf_release(&of->t_path);
  strbuf_release(&of->dir_path);
  strbuf_release(&of->obj_path);
  strbuf_release(&of->metadata);

  of->t_size = 0;
  char blank_hash[EVP_MAX_MD_SIZE];
  memcpy(of->hash, blank_hash, EVP_MAX_MD_SIZE);
}

// ----

int get_object_path(char buffer[], const char *hash) {
  int p_size =
      snprintf(buffer, PATH_MAX, ".tig/objects/%.2s/%s", hash, hash + 2);

  if ((unsigned long)p_size >= PATH_MAX) {
    error("Provided path is too long. Max path size is: %d", PATH_MAX);
    return -1;
  }
  return 0;
}

int decompress_object_file(const char *hash) {
  struct object obj;
  read_from_database(&obj, hash);

  char path[PATH_MAX] = {0};
  if (get_object_path(path, hash) != 0) {
    return -1;
  }

  struct strbuf sb = STRBUF_INIT;

  if (decompress_file(&sb, path) != 0) {
    strbuf_release(&sb);
    return -1;
  }

  char *removed_meta_obj = strchr(sb.buf, '\0') + 1;
  fwrite(removed_meta_obj, 1, sb.len, stdout);

  strbuf_release(&sb);

  return 0;
}

int compress_file_to_obj_file(struct object_file *of, const char *path) {
  create_dir(of->dir_path.buf);

  if (compress_to_file(of->metadata.buf, path, of->obj_path.buf) != 0) {
    return -1;
  }

  return 0;
}

int hash_object_file(struct object_file *of, const char *path) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    return -1;
  }

  unsigned char hash[EVP_MAX_MD_SIZE];
  unsigned char in_buffer[CHUNK];
  size_t bytes_read = 0;
  EVP_MD_CTX *hashctx = EVP_MD_CTX_new();

  EVP_MD_CTX_init(hashctx);
  EVP_DigestInit_ex(hashctx, EVP_sha1(), NULL);

  EVP_DigestUpdate(hashctx, of->metadata.buf, strlen(of->metadata.buf) + 1);

  do {
    bytes_read = fread(in_buffer, 1, CHUNK, file);
    if (!bytes_read) {
      break;
    }

    EVP_DigestUpdate(hashctx, in_buffer, bytes_read);
  } while (bytes_read == CHUNK);

  unsigned int outlen;
  EVP_DigestFinal_ex(hashctx, hash, &outlen);
  EVP_MD_CTX_free(hashctx);
  fclose(file);

  char str_hash[EVP_MAX_MD_SIZE];
  for (unsigned int i = 0; i < outlen; i++) {
    snprintf(&str_hash[i * 2], 3, "%02x", hash[i]);
  }
  memcpy(of->hash, &str_hash, EVP_MAX_MD_SIZE);

  strbuf_addf(&of->dir_path, ".tig/objects/%.2s", str_hash);
  strbuf_addf(&of->obj_path, ".tig/objects/%.2s/%s", str_hash, str_hash + 2);

  return 0;
}
