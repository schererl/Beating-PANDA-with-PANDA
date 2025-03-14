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

#ifndef __BOR_CARR_H__
#define __BOR_CARR_H__

#include <string.h>
#include <boruvka/compiler.h>
#include <boruvka/alloc.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

struct bor_carr {
    char *arr;
    int size;
    int alloc;
};
typedef struct bor_carr bor_carr_t;

#define BOR_CARR_INIT { NULL, 0, 0 }
#define BOR_CARR(NAME) bor_carr_t NAME = BOR_CARR_INIT

#define BOR_CARR_FOR_EACH(S, V) \
    for (int __i = 0; __i < (S)->size && ((V) = (S)->arr[__i], 1); ++__i)

/**
 * Initialize the array.
 */
_bor_inline void borCArrInit(bor_carr_t *a);

/**
 * Frees allocated memory.
 */
_bor_inline void borCArrFree(bor_carr_t *a);

/**
 * Allocate enough memory for size elements.
 * (It does not change a->size.)
 */
_bor_inline void borCArrRealloc(bor_carr_t *a, int size);

/**
 * Resize the array to the specified number of elements.
 */
_bor_inline void borCArrResize(bor_carr_t *a, int size);

/**
 * Returns ith element from the array.
 */
_bor_inline char borCArrGet(const bor_carr_t *a, int i);

/**
 * Sets ith element from the array.
 */
_bor_inline void borCArrSet(bor_carr_t *a, int i, char val);

/**
 * Returns size of the array.
 */
_bor_inline int borCArrSize(const bor_carr_t *a);

/**
 * Returns array stored in the structure.
 */
_bor_inline char *borCArrGetArr(const bor_carr_t *a);


/**
 * Returns true if val \in s
 */
_bor_inline int borCArrIn(char val, const bor_carr_t *s);

/**
 * Makes the array empty.
 */
_bor_inline void borCArrEmpty(bor_carr_t *s);

/**
 * Adds element at the end of the array.
 */
_bor_inline void borCArrAdd(bor_carr_t *s, char val);

/**
 * Adds element at the begginig of the array.
 */
_bor_inline void borCArrPrepend(bor_carr_t *s, char val);

/**
 * Returns true if the array are equal.
 */
_bor_inline int borCArrEq(const bor_carr_t *s1, const bor_carr_t *s2);

/**
 * Compares arrays, return values are the same as by memcmp().
 */
_bor_inline int borCArrCmp(const bor_carr_t *s1, const bor_carr_t *s2);



/**** INLINES: ****/
_bor_inline void borCArrInit(bor_carr_t *a)
{
    bzero(a, sizeof(*a));
}

_bor_inline void borCArrFree(bor_carr_t *a)
{
    if (a->arr != NULL)
        BOR_FREE(a->arr);
}

_bor_inline void borCArrRealloc(bor_carr_t *a, int size)
{
    while (size > a->alloc){
        if (a->alloc == 0)
            a->alloc = 1;
        a->alloc *= 2;
    }
    a->arr = BOR_REALLOC_ARR(a->arr, char, a->alloc);
}

_bor_inline void borCArrResize(bor_carr_t *a, int size)
{
    if (size > a->alloc)
        borCArrRealloc(a, size);
    a->size = size;
}

_bor_inline char borCArrGet(const bor_carr_t *a, int i)
{
    return a->arr[i];
}

_bor_inline void borCArrSet(bor_carr_t *a, int i, char val)
{
    if (a->size <= i)
        borCArrResize(a, i + 1);
    a->arr[i] = val;
}

_bor_inline int borCArrSize(const bor_carr_t *a)
{
    return a->size;
}

_bor_inline char *borCArrGetArr(const bor_carr_t *a)
{
    return a->arr;
}

_bor_inline int borCArrIn(char val, const bor_carr_t *a)
{
    for (int i = 0; i < a->size; ++i){
        if (a->arr[i] == val)
            return 1;
    }
    return 0;
}

_bor_inline void borCArrEmpty(bor_carr_t *a)
{
    a->size = 0;
}

_bor_inline void borCArrAdd(bor_carr_t *a, char val)
{
    if (a->size == a->alloc){
        if (a->alloc == 0)
            a->alloc = 1;
        a->alloc *= 2;
        a->arr = BOR_REALLOC_ARR(a->arr, char, a->alloc);
    }
    a->arr[a->size++] = val;
}

_bor_inline void borCArrPrepend(bor_carr_t *a, char val)
{
    if (a->size == a->alloc){
        if (a->alloc == 0)
            a->alloc = 1;
        a->alloc *= 2;
        a->arr = BOR_REALLOC_ARR(a->arr, char, a->alloc);
    }
    for (int i = a->size; i > 0; --i)
        a->arr[i] = a->arr[i - 1];
    ++a->size;
    a->arr[0] = val;
}

_bor_inline int borCArrEq(const bor_carr_t *s1, const bor_carr_t *s2)
{
    return s1->size == s2->size
            && memcmp(s1->arr, s2->arr, sizeof(char) * s1->size) == 0;
}

_bor_inline int borCArrCmp(const bor_carr_t *s1, const bor_carr_t *s2)
{
    int cmp;
    cmp = memcmp(s1->arr, s2->arr,
                 sizeof(char) * (s1->size < s2->size ?  s1->size : s2->size));
    if (cmp == 0)
        return s1->size - s2->size;
    return cmp;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_CARR_H__ */
