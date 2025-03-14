/***
 * Boruvka
 * --------
 * Copyright (c)2017 Daniel Fiser <danfis@danfis.cz>
 *
 *  This file is part of Boruvka.
 *
 *  Distributed under the OSI-approved BSD License (the "License");
 *  see accompanying file BDS-LICENSE for details or see
 *  <http://www.opensource.org/licenses/bsd-license.php>.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even the
 *  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *  See the License for more information.
 */

#ifndef __BOR_LARR_H__
#define __BOR_LARR_H__

#include <string.h>
#include <boruvka/compiler.h>
#include <boruvka/alloc.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct bor_larr {
    long *arr;
    int size;
    int alloc;
};
typedef struct bor_larr bor_larr_t;

#define BOR_LARR_INIT { NULL, 0, 0 }
#define BOR_LARR(NAME) bor_larr_t NAME = BOR_LARR_INIT

#define BOR_LARR_FOR_EACH(S, V) \
    for (int __i = 0; __i < (S)->size && ((V) = (S)->arr[__i], 1); ++__i)

/**
 * Initialize the array.
 */
_bor_inline void borLArrInit(bor_larr_t *a);

/**
 * Frees allocated memory.
 */
_bor_inline void borLArrFree(bor_larr_t *a);

/**
 * Allocate enough memory for size elements.
 * (It does not change a->size.)
 */
_bor_inline void borLArrRealloc(bor_larr_t *a, int size);

/**
 * Resize the array to the specified number of elements.
 */
_bor_inline void borLArrResize(bor_larr_t *a, int size);

/**
 * Returns ith element from the array.
 */
_bor_inline long borLArrGet(const bor_larr_t *a, int i);

/**
 * Sets ith element from the array.
 */
_bor_inline void borLArrSet(bor_larr_t *a, int i, long val);

/**
 * Returns size of the array.
 */
_bor_inline int borLArrSize(const bor_larr_t *a);

/**
 * Returns array stored in the structure.
 */
_bor_inline long *borLArrGetArr(const bor_larr_t *a);


/**
 * Returns true if val \in s
 */
_bor_inline int borLArrIn(long val, const bor_larr_t *s);

/**
 * Makes the array empty.
 */
_bor_inline void borLArrEmpty(bor_larr_t *s);

/**
 * Adds element at the end of the array.
 */
_bor_inline void borLArrAdd(bor_larr_t *s, long val);

/**
 * Adds element at the begginig of the array.
 */
_bor_inline void borLArrPrepend(bor_larr_t *s, long val);

/**
 * Returns true if the array are equal.
 */
_bor_inline int borLArrEq(const bor_larr_t *s1, const bor_larr_t *s2);

/**
 * Compares arrays, return values are the same as by memcmp().
 */
_bor_inline int borLArrCmp(const bor_larr_t *s1, const bor_larr_t *s2);



/**** INLINES: ****/
_bor_inline void borLArrInit(bor_larr_t *a)
{
    bzero(a, sizeof(*a));
}

_bor_inline void borLArrFree(bor_larr_t *a)
{
    if (a->arr != NULL)
        BOR_FREE(a->arr);
}

_bor_inline void borLArrRealloc(bor_larr_t *a, int size)
{
    while (size > a->alloc){
        if (a->alloc == 0)
            a->alloc = 1;
        a->alloc *= 2;
    }
    a->arr = BOR_REALLOC_ARR(a->arr, long, a->alloc);
}

_bor_inline void borLArrResize(bor_larr_t *a, int size)
{
    if (size > a->alloc)
        borLArrRealloc(a, size);
    a->size = size;
}

_bor_inline long borLArrGet(const bor_larr_t *a, int i)
{
    return a->arr[i];
}

_bor_inline void borLArrSet(bor_larr_t *a, int i, long val)
{
    if (a->size <= i)
        borLArrResize(a, i + 1);
    a->arr[i] = val;
}

_bor_inline int borLArrSize(const bor_larr_t *a)
{
    return a->size;
}

_bor_inline long *borLArrGetArr(const bor_larr_t *a)
{
    return a->arr;
}

_bor_inline int borLArrIn(long val, const bor_larr_t *a)
{
    for (int i = 0; i < a->size; ++i){
        if (a->arr[i] == val)
            return 1;
    }
    return 0;
}

_bor_inline void borLArrEmpty(bor_larr_t *a)
{
    a->size = 0;
}

_bor_inline void borLArrAdd(bor_larr_t *a, long val)
{
    if (a->size == a->alloc){
        if (a->alloc == 0)
            a->alloc = 1;
        a->alloc *= 2;
        a->arr = BOR_REALLOC_ARR(a->arr, long, a->alloc);
    }
    a->arr[a->size++] = val;
}

_bor_inline void borLArrPrepend(bor_larr_t *a, long val)
{
    if (a->size == a->alloc){
        if (a->alloc == 0)
            a->alloc = 1;
        a->alloc *= 2;
        a->arr = BOR_REALLOC_ARR(a->arr, long, a->alloc);
    }
    for (int i = a->size; i > 0; --i)
        a->arr[i] = a->arr[i - 1];
    ++a->size;
    a->arr[0] = val;
}

_bor_inline int borLArrEq(const bor_larr_t *s1, const bor_larr_t *s2)
{
    return s1->size == s2->size
            && memcmp(s1->arr, s2->arr, sizeof(long) * s1->size) == 0;
}

_bor_inline int borLArrCmp(const bor_larr_t *s1, const bor_larr_t *s2)
{
    int cmp;
    cmp = memcmp(s1->arr, s2->arr,
                 sizeof(long) * (s1->size < s2->size ?  s1->size : s2->size));
    if (cmp == 0)
        return s1->size - s2->size;
    return cmp;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_LARR_H__ */
