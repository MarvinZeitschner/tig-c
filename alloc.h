#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

#ifndef ALLOC_H
#define ALLOC_H

/**
 * ALLOC_GROW - Macro to dynamically grow memory
 *
 * @p: Pointer to memory to be grown
 * @nr: New size of the memory
 * @alloc: Pointer to variable that holds currently allocated space
 *
 * @returns: -1 if the memory was grown, 0 if it wasn't
 */
#define ALLOC_GROW(p, nr, alloc) \
  (-!((nr) <= (alloc) ||         \
      ((p) = alloc_grow_helper((p), (nr), &(alloc), sizeof(*(p))))))

/**
 * alloc_grow_helper - Function used in ALLOC_GROW to grow given value
 *
 * @p: Pointer to memory to be grown
 * @nr: Required new size of the buffer
 * @alloc: Pointer to a variable that holds current allocated size, which will
 * be updated
 * @size: Size of each individual element in buffer
 */
void* alloc_grow_helper(void* p, long nr, unsigned long* alloc, size_t size);

#endif
