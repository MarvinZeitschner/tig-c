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

void get_object_path(char buffer[], const char *hash) {
  int p_size =
      snprintf(buffer, PATH_MAX, ".tig/objects/%.2s/%s", hash, hash + 2);

  if ((unsigned long)p_size >= PATH_MAX) {
    die("Provided path is too long. Max path size is: %d", PATH_MAX);
  }
}

void decompress_object_file(const char *hash) {
  char path[PATH_MAX] = {'\0'};
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

// TODO: add buffer param to return hash, bc of -w flag
void hash_object_file(const char *path) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    die("Failed to open file: %s", path);
  }
  struct stat st;

  if (stat(path, &st) != 0) {
    die("Failed to get metadata from file: %s", path);
  }

  /**
   * Is strbuf more useful here?
   * If not: TODO: boundary check, bc the snprintf shouldnt cut off data we need
   * in the compression
   */
  char metadata[METADATA_MAX] = "\0";
  snprintf(metadata, METADATA_MAX, "blob %lld", st.st_size);

  unsigned char hash[EVP_MAX_MD_SIZE] = "\0";
  unsigned char in_buffer[CHUNK] = "\0";
  size_t bytes_read = 0;
  EVP_MD_CTX *hashctx = EVP_MD_CTX_new();

  EVP_MD_CTX_init(hashctx);
  EVP_DigestInit_ex(hashctx, EVP_sha1(), NULL);

  // TODO: Boundary check
  EVP_DigestUpdate(hashctx, metadata, strlen(metadata) + 1);

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
  // fclose(file);

  char str_hash[EVP_MAX_MD_SIZE];
  for (unsigned int i = 0; i < outlen; i++) {
    snprintf(&str_hash[i * 2], 3, "%02x", hash[i]);
  }
  printf("SHA-1 hash: %s\n", str_hash);

  char dir_path[PATH_MAX];
  char obj_path[PATH_MAX];
  /**
   * TODO: boundary checks
   * we do not want snprintf to truncate the object file paths
   * maybe strbuf really is the better approach
   */
  snprintf(dir_path, PATH_MAX, ".tig/objects/%.2s", str_hash);
  snprintf(obj_path, PATH_MAX, ".tig/objects/%.2s/%s", str_hash, str_hash + 2);
  printf("dir_path: %s\n", dir_path);
  printf("obj_path: %s\n", obj_path);

  // --------------- zlib magic --------------- //
  rewind(file);

  FILE *obj_file = fopen(obj_path, "wb");
  if (!obj_file) {
    die("Failed to open file %s: %s", obj_path, strerror(errno));
  }

  int ret, flush;
  unsigned have;
  z_stream strm = {0};
  unsigned char in[CHUNK] = {"\0"};
  unsigned char out[CHUNK] = {"\0"};

  ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
  if (ret != Z_OK) {
    fclose(file);
    fclose(obj_file);
    die("Failed to initialize zlib");
  }
  /**
   * Append the metadata to the in buffer to get compressed
   */
  strcpy((char *)in, metadata);
  strcat((char *)in, "\0");
  strm.avail_in = strlen((char *)in) + 1;
  strm.next_in = in;
  strm.avail_out = CHUNK;
  strm.next_out = out;
  ret = deflate(&strm, Z_NO_FLUSH);
  assert(ret != Z_STREAM_ERROR);
  have = CHUNK - strm.avail_out;
  if (fwrite(out, 1, have, obj_file) != have || ferror(obj_file)) {
    (void)deflateEnd(&strm);
    // return Z_ERRNO;
  }

  do {
    strm.avail_in = fread(in, 1, CHUNK, file);
    if (ferror(file)) {
      perror("");
      (void)deflateEnd(&strm);
      fclose(file);
      fclose(obj_file);
      // return Z_ERRNO;
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
        // return Z_ERRNO;
      }
    } while (strm.avail_out == 0);
  } while (flush != Z_FINISH);
  assert(ret == Z_STREAM_END);

  (void)deflateEnd(&strm);
  fclose(file);
  fclose(obj_file);

  //
  //
  //
  // return Z_OK;

  // create_dir(dir_path);

  // compress_buf(metadata, strlen(metadata) + 1, obj_path);
  // compress_file(path, obj_path);

  return;
}

int compress_buf(const char *buf, size_t size, const char *path_to_write) {
  int ret;
  unsigned have;
  z_stream strm = {0};
  unsigned char out[CHUNK];

  FILE *obj_file = fopen(path_to_write, "wb");
  if (!obj_file) {
    fprintf(stderr, "Failed to open file %s: %s\n", path_to_write,
            strerror(errno));
    return Z_ERRNO;
  }

  ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
  if (ret != Z_OK) {
    fprintf(stderr, "Failed to initialize zlib: %d\n", ret);
    fclose(obj_file);
    return ret;
  }

  strm.next_in = (unsigned char *)buf;
  strm.avail_in = size;

  do {
    strm.avail_out = CHUNK;
    strm.next_out = out;

    ret = deflate(&strm, Z_FINISH);
    if (ret != Z_OK && ret != Z_STREAM_END) {
      fprintf(stderr, "Error during compression: %d\n", ret);
      (void)deflateEnd(&strm);
      fclose(obj_file);
      return ret;
    }

    have = CHUNK - strm.avail_out;
    if (fwrite(out, 1, have, obj_file) != have || ferror(obj_file)) {
      fprintf(stderr, "Error writing to file %s\n", path_to_write);
      (void)deflateEnd(&strm);
      fclose(obj_file);
      return Z_ERRNO;
    }
  } while (ret != Z_STREAM_END);

  (void)deflateEnd(&strm);
  fclose(obj_file);

  return Z_OK;
}

int compress_file(const char *path, const char *path_to_write) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    fclose(file);
  }
  FILE *obj_file = fopen(path_to_write, "ab");
  if (!obj_file) {
    die("Failed to open file %s: %s", path_to_write, strerror(errno));
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
    die("Failed to initialize zlib");
  }
  do {
    strm.avail_in = fread(in, 1, CHUNK, file);
    if (ferror(file)) {
      perror("");
      (void)deflateEnd(&strm);
      fclose(file);
      fclose(obj_file);
      return Z_ERRNO;
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
        return Z_ERRNO;
      }
    } while (strm.avail_out == 0);
  } while (flush != Z_FINISH);
  assert(ret == Z_STREAM_END);

  (void)deflateEnd(&strm);
  fclose(file);
  fclose(obj_file);

  return Z_OK;
}
