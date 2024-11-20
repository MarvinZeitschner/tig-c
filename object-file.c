#include "object-file.h"

#include <assert.h>
#include <errno.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "error.h"
#include "path.h"
#include "strbuf.h"

void get_object_path(char buffer[], const char* hash) {
  int p_size =
      snprintf(buffer, PATH_MAX, ".tig/objects/%.2s/%s", hash, hash + 2);

  if ((unsigned long)p_size >= PATH_MAX) {
    die("Provided path is too long. Max path size is: %d", PATH_MAX);
  }
}

void decompress_object_file(const char* hash) {
  char path[PATH_MAX] = {'\0'};
  get_object_path(path, hash);

  FILE* file = fopen(path, "rb");
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
      strbuf_addstr(&sb, (char*)out_buffer, CHUNK - strm.avail_out);
    } while (strm.avail_out == 0);
    /**
     *  We're done when inflate() return Z_STREAM_END
     */
  } while (ret != Z_STREAM_END);
  char* removed_meta_obj = strchr(sb.buf, '\0') + 1;
  fwrite(removed_meta_obj, 1, sb.len, stdout);
  (void)inflateEnd(&strm);

  strbuf_release(&sb);

  return;
}

void hash_object_file(const char* path) {
  FILE* file = fopen(path, "r");
  if (!file) {
    die("Failed to open file: %s", path);
  }

  unsigned char hash[EVP_MAX_MD_SIZE] = "\0";
  unsigned char in_buffer[CHUNK] = "\0";
  size_t bytes_read = 0;
  EVP_MD_CTX* hashctx = EVP_MD_CTX_new();

  EVP_MD_CTX_init(hashctx);
  EVP_DigestInit_ex(hashctx, EVP_sha1(), NULL);

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

  printf("SHA-1 hash: ");
  char test[EVP_MAX_MD_SIZE];
  for (unsigned int i = 0; i < outlen; i++) {
    snprintf(test + strlen(test), EVP_MAX_MD_SIZE, "%02x", hash[i]);
  }
  printf("%s\n", test);
  return;
}
