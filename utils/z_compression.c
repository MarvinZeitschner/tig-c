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
#include "strbuf.h"
#include "z_compression.h"

int decompress_file(struct strbuf *sb, char *path) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    error("Failed to open file %s: %s", path, strerror(errno));
    return -1;
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

  int ret = inflateInit(&strm);
  if (ret != Z_OK) {
    fclose(file);
    error("Failed to initialize zlib stream");
    return -1;
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
      fclose(file);
      (void)inflateEnd(&strm);
      error("Error handling file stream in decompression");
      return -1;
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
      switch (ret) {
      case Z_NEED_DICT:
        ret = Z_DATA_ERROR;
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
      case Z_STREAM_ERROR:
        fclose(file);
        (void)inflateEnd(&strm);
        return -1;
      }
      strbuf_addstr(sb, (char *)out_buffer, CHUNK - strm.avail_out);
    } while (strm.avail_out == 0);
    /**
     *  We're done when inflate() return Z_STREAM_END
     */
  } while (ret != Z_STREAM_END);

  (void)inflateEnd(&strm);
  fclose(file);

  return 0;
}

int decompress_file_metadata(struct strbuf *sb, char *path) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    error("Failed to open file %s: %s", path, strerror(errno));
    return -1;
  }

  z_stream strm = {0};
  unsigned char in_buffer[CHUNK];
  unsigned char out_buffer[CHUNK];

  int ret = inflateInit(&strm);
  if (ret != Z_OK) {
    fclose(file);
    error("Failed to initialize zlib stream");
    return -1;
  }

  do {
    strm.avail_in = fread(in_buffer, 1, CHUNK, file);
    if (ferror(file)) {
      fclose(file);
      (void)inflateEnd(&strm);
      error("Error handling file stream in decompression");
      return -1;
    }
    if (strm.avail_in == 0) {
      break;
    }
    strm.next_in = in_buffer;

    do {
      strm.avail_out = CHUNK;
      strm.next_out = out_buffer;
      ret = inflate(&strm, Z_NO_FLUSH);
      switch (ret) {
      case Z_NEED_DICT:
        ret = Z_DATA_ERROR;
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
      case Z_STREAM_ERROR:
        fclose(file);
        (void)inflateEnd(&strm);
        return -1;
      }
      char *null_pos = memchr(out_buffer, '\0', CHUNK - strm.avail_out);
      if (null_pos) {
        size_t len = null_pos - (char *)out_buffer;
        strbuf_addstr(sb, (char *)out_buffer, len);
        (void)inflateEnd(&strm);
        fclose(file);
        return 1;
      }
      strbuf_addstr(sb, (char *)out_buffer, CHUNK - strm.avail_out);
    } while (strm.avail_out == 0);
  } while (ret != Z_STREAM_END);

  (void)inflateEnd(&strm);
  fclose(file);

  return 0;
}

int compress_to_file(char *metadata, const char *path, char *path_to_write) {
  FILE *file = fopen(path, "rb");
  if (!file) {
    perror("");
    return -1;
  }

  FILE *obj_file = fopen(path_to_write, "wb");
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
  if (metadata != NULL) {
    strcpy((char *)in, metadata);
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
