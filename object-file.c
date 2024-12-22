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
#include "path.h"
#include "strbuf.h"

void object_file_init(struct object_file *of) {
  struct object_file blank = {0};

  strbuf_init(&blank.t_path, 0);
  strbuf_init(&blank.dir_path, 0);
  strbuf_init(&blank.obj_path, 0);
  strbuf_init(&blank.metadata, 0);

  // strbuf_addstr(&blank.t_path, path, sizeof(path));

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

void get_object_path(char buffer[], const char *hash) {
  int p_size =
      snprintf(buffer, PATH_MAX, ".tig/objects/%.2s/%s", hash, hash + 2);

  if ((unsigned long)p_size >= PATH_MAX) {
    die("Provided path is too long. Max path size is: %d", PATH_MAX);
  }
}

void decompress_object_file(const char *hash) {
  char path[PATH_MAX] = {0};
  get_object_path(path, hash);

  FILE *file = fopen(path, "rb");
  if (!file) {
    die("Failed to open file %s: %s", path, strerror(errno));
  }

  /**
   * Initialize the z_stream stream and set it to null.
   * If inflateInit fails we return as no decompression can be performed
   * inflateInit provides access to information about compression to
   * aid in memory allocation
   */
  z_stream strm = {0};
  unsigned char in_buffer[CHUNK];
  unsigned char out_buffer[CHUNK];
  struct strbuf sb = STRBUF_INIT;

  int ret = inflateInit(&strm);
  if (ret != Z_OK) {
    die("Failed to initialize zlib stream");
  }

  /**
   * Decompress input with given CHUNK-buffer until inflate() indicates
   * it has reached an end of compressed data
   */
  do {
    /**
     * Read one byte at a time for the size of CHUNK of file.
     * If some fileoperation fails, terminate.
     * avail_in tells us how many bytes are left to process in the input buffer.
     */
    strm.avail_in = fread(in_buffer, 1, CHUNK, file);
    if (ferror(file)) {
      (void)inflateEnd(&strm);
      die("Error handling file stream in decompression");
    }
    /**
     * avail_in == 0 is equivalient to no more input data being left to process.
     */
    if (strm.avail_in == 0) {
      break;
    }
    /**
     * next_in points to the current position in the input-buffer where
     * decompression should start/continue.
     */
    strm.next_in = in_buffer;

    /**
     * run inflate() on input-buffer until output of buffer is empty
     * so that we continue decompressing with the buffer.
     */
    do {
      strm.avail_out = CHUNK;
      strm.next_out = out_buffer;
      ret = inflate(&strm, Z_NO_FLUSH);
      assert(ret != Z_STREAM_ERROR);
      switch (ret) {
      case Z_NEED_DICT:
        ret = Z_DATA_ERROR;
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
        (void)inflateEnd(&strm);
        die("zlib error: %d", ret);
      }
      strbuf_addstr(&sb, (char *)out_buffer, CHUNK - strm.avail_out);
    } while (strm.avail_out == 0);
    /**
     *  We're done when inflate() return Z_STREAM_END
     */
  } while (ret != Z_STREAM_END);
  char *removed_meta_obj = strchr(sb.buf, '\0') + 1;
  fwrite(removed_meta_obj, 1, sb.len, stdout);
  (void)inflateEnd(&strm);

  strbuf_release(&sb);

  return;
}

int compress_file_to_obj_file(struct object_file *of, const char *path) {
  create_dir(of->dir_path.buf);

  FILE *file = fopen(path, "rb");
  if (!file) {
    perror("");
    return -1;
  }

  FILE *obj_file = fopen(of->obj_path.buf, "wb");
  if (!obj_file) {
    fclose(file);
    perror("");
    return -1;
  }

  int ret, flush;
  unsigned have;
  z_stream strm = {0};
  unsigned char in[CHUNK];
  unsigned char out[CHUNK];

  ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
  if (ret != Z_OK) {
    fclose(file);
    fclose(obj_file);
    error("Failed to initialize zlib");
    return -1;
  }
  /**
   * Append the metadata to the in buffer to get compressed
   */
  strcpy((char *)in, of->metadata.buf);
  strcat((char *)in, "\0");
  strm.avail_in = strlen((char *)in) + 1;
  strm.next_in = in;
  strm.avail_out = CHUNK;
  strm.next_out = out;
  ret = deflate(&strm, Z_NO_FLUSH);
  if (ret == Z_STREAM_ERROR) {
    fclose(file);
    fclose(obj_file);
    die("A zlib error occurred");
  }
  have = CHUNK - strm.avail_out;
  if (fwrite(out, 1, have, obj_file) != have || ferror(obj_file)) {
    (void)deflateEnd(&strm);
    fclose(file);
    fclose(obj_file);
    return -1;
  }

  do {
    strm.avail_in = fread(in, 1, CHUNK, file);
    if (ferror(file)) {
      perror("");
      (void)deflateEnd(&strm);
      fclose(file);
      fclose(obj_file);
      return -1;
    }
    flush = feof(file) ? Z_FINISH : Z_NO_FLUSH;
    strm.next_in = in;

    do {
      strm.avail_out = CHUNK;
      strm.next_out = out;
      ret = deflate(&strm, flush);
      assert(ret != Z_STREAM_ERROR);
      have = CHUNK - strm.avail_out;
      if (fwrite(out, 1, have, obj_file) != have || ferror(obj_file)) {
        (void)deflateEnd(&strm);
        fclose(file);
        fclose(obj_file);
        return -1;
      }
    } while (strm.avail_out == 0);
  } while (flush != Z_FINISH);
  assert(ret == Z_STREAM_END);

  (void)deflateEnd(&strm);
  fclose(file);
  fclose(obj_file);

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
