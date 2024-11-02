#include "object-file.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

#include "path.h"

void decompress_object_file(char* hash) {
  char path[PATH_MAX];
  int p_size =
      snprintf(path, sizeof(path), ".tig/objects/%.2s/%s", hash, hash + 2);

  if ((unsigned long)p_size >= sizeof(path)) {
    printf("Provided path is too long. Max path size: %d\n", PATH_MAX);
    return;
  }

  FILE* file = fopen(path, "rb");
  if (!file) {
    perror("Failed to open object file");
    return;
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
    return;
  }

  /**
   * Decompress input with given CHUNK-buffer until inflate() indicates
   * it has reached an end of compressed data
   */
  do {
    /**
     * Read on bit at a time for the size of CHUNK of file.
     * If some fileoperation fails, terminate.
     * avail_in tells us how many bytes are left to process in the input buffer.
     */
    strm.avail_in = fread(in_buffer, 1, CHUNK, file);
    if (ferror(file)) {
      (void)inflateEnd(&strm);
      fprintf(stderr, "Error handling file stream in decompression");
      return;
    }
    /**
     * avail_in == 0 is euqivalent to the output-buffer being full
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
          return;
      }
      fwrite(out_buffer, 1, CHUNK - strm.avail_out, stdout);
    } while (strm.avail_out == 0);
    /**
     *  We're done when inflate() return Z_STREAM_END
     */
  } while (ret != Z_STREAM_END);
  (void)inflateEnd(&strm);
  return;
}
