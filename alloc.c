#include <limits.h>
#include <stdlib.h>

/**
 * alloc_grow_helper - Function used in ALLOC_GROW to grow given value
 *
 * @p: Pointer to memory to be grown
 * @nr: Required new size of the buffer
 * @alloc: Pointer to a variable that holds current allocated size, which will
 * be updated
 * @size: Size of each individual element in buffer
 */
void* alloc_grow_helper(void* p, long nr, unsigned long* alloc, size_t size) {
  void* tmp = NULL;

  // Check the upper bounds so that there isn't an overflow
  size_t n = ((LONG_MAX - 16) / 2 >= *alloc) ? 2 * *alloc + 16 : LONG_MAX;
  /**
   * If the given required size is bigger than the above n, assign n to the give
   * size
   */
  if (nr > n) {
    n = nr;
  }
  if (SIZE_MAX / size >= n) {
    tmp = realloc(p, n * size);
  } else {
    free(p);
    *alloc = 0;
  }
  return tmp;
}
