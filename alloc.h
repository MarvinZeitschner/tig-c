#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "error.h"

#ifndef ALLOC_H
#define ALLOC_H

/**
 * ALLOC_GROW - Macro to dynamically grow memory
 * 
 * @p: Pointer to memory to be grown
 * @nr: New size of the memory
 * @alloc: Pointer to variable that holds currently allocated space
 */
#define ALLOC_GROW(p, nr, alloc) \
    (-!((nr) <= (alloc) ||      \
        ((p) = alloc_grow_helper((p), (nr), &(alloc), sizeof(*(p))))))

void* alloc_grow_helper(void* p, long nr, unsigned long *alloc, size_t size);

#endif
