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

#ifndef __BOR_LSET_H__
#define __BOR_LSET_H__

#include <string.h>
#include <boruvka/compiler.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Array-based set.
 * The elements in .s are always sorted.
 */
struct bor_lset {
    long *s;
    int size;
    int alloc;
};
typedef struct bor_lset bor_lset_t;

#define BOR_LSET_INIT { NULL, 0, 0 }
#define BOR_LSET(NAME) bor_lset_t NAME = BOR_LSET_INIT

#define BOR_LSET_FOR_EACH(S, V) \
    for (int __i = 0; __i < (S)->size && ((V) = (S)->s[__i], 1); ++__i)

#define BOR_LSET_ADD(S, ...) \
    do { \
        long ___bor_lset_vals[] = {__VA_ARGS__}; \
        int ___bor_lset_size = sizeof(___bor_lset_vals) / sizeof(long); \
        for (int i = 0; i < ___bor_lset_size; ++i) \
            borLSetAdd((S), ___bor_lset_vals[i]); \
    }while (0)

#define BOR_LSET_SET(S, ...) \
    do { \
        borLSetEmpty(S); \
        BOR_LSET_ADD((S), __VA_ARGS__); \
    } while (0)

/**
 * Initialize the set.
 */
void borLSetInit(bor_lset_t *s);

/**
 * Frees allocated memory.
 */
void borLSetFree(bor_lset_t *s);

/**
 * Returns ith element from the set.
 */
_bor_inline long borLSetGet(const bor_lset_t *s, int i);

/**
 * Returns size of the set.
 */
_bor_inline int borLSetSize(const bor_lset_t *s);


/**
 * Returns true if val \in s
 */
int borLSetHas(const bor_lset_t *s, long val);
_bor_inline int borLSetIn(long val, const bor_lset_t *s);

/**
 * Return true if s1 \subset s2
 */
int borLSetIsSubset(const bor_lset_t *s1, const bor_lset_t *s2);

/**
 * Returns size of s1 \cap s2.
 */
int borLSetIntersectionSize(const bor_lset_t *s1, const bor_lset_t *s2);

/**
 * Returns true if | s1 \cap s2 | >= limit
 */
int borLSetIntersectionSizeAtLeast(const bor_lset_t *s1, const bor_lset_t *s2,
                                  int limit);

/**
 * Returns true if | s1 \cap s2 \cap s3 | >= limit
 */
int borLSetIntersectionSizeAtLeast3(const bor_lset_t *s1,
                                   const bor_lset_t *s2,
                                   const bor_lset_t *s3,
                                   int limit);

/**
 * Returns true if the sets are disjunct.
 */
_bor_inline int borLSetIsDisjunct(const bor_lset_t *s1, const bor_lset_t *s2);

/**
 * s = \emptyset
 */
_bor_inline void borLSetEmpty(bor_lset_t *s);

/**
 * d = s
 */
void borLSetSet(bor_lset_t *d, const bor_lset_t *s);

/**
 * s = s \cup {val}
 */
void borLSetAdd(bor_lset_t *s, long val);

/**
 * s = s \setminus {val}
 * Returns true if val was found in s.
 */
int borLSetRm(bor_lset_t *s, long val);

/**
 * dst = dst \cup src
 */
void borLSetUnion(bor_lset_t *dst, const bor_lset_t *src);

/**
 * dst = s1 \cup s2
 */
void borLSetUnion2(bor_lset_t *dst, const bor_lset_t *s1, const bor_lset_t *s2);

/**
 * dst = dst \cap src
 */
void borLSetIntersect(bor_lset_t *dst, const bor_lset_t *src);

/**
 * dst = s1 \cap s2
 */
void borLSetIntersect2(bor_lset_t *dst, const bor_lset_t *s1, const bor_lset_t *s2);

/**
 * s1 = s1 \setminus s2
 */
void borLSetMinus(bor_lset_t *s1, const bor_lset_t *s2);

/**
 * d = s1 \setminus s2
 */
void borLSetMinus2(bor_lset_t *d, const bor_lset_t *s1, const bor_lset_t *s2);


/**
 * Returns true if the sets are equal.
 */
_bor_inline int borLSetEq(const bor_lset_t *s1, const bor_lset_t *s2);

/**
 * Compares sets, return values are the same as by memcmp().
 */
_bor_inline int borLSetCmp(const bor_lset_t *s1, const bor_lset_t *s2);

/**
 * Remaps the elements of the set using remap array containing maping from
 * the old value to the new value. The mapping must be monotonically
 * increasing and it is assumed that the values in the set are >= 0.
 */
void borLSetRemap(bor_lset_t *s, const long *remap);



/**** INLINES: ****/
_bor_inline long borLSetGet(const bor_lset_t *s, int i)
{
    return s->s[i];
}

_bor_inline int borLSetSize(const bor_lset_t *s)
{
    return s->size;
}

_bor_inline int borLSetIn(long val, const bor_lset_t *s)
{
    return borLSetHas(s, val);
}

_bor_inline int borLSetIsDisjunct(const bor_lset_t *s1, const bor_lset_t *s2)
{
    return !borLSetIntersectionSizeAtLeast(s1, s2, 1);
}

_bor_inline void borLSetEmpty(bor_lset_t *s)
{
    s->size = 0;
}

_bor_inline int borLSetEq(const bor_lset_t *s1, const bor_lset_t *s2)
{
    return s1->size == s2->size
            && memcmp(s1->s, s2->s, sizeof(long) * s1->size) == 0;
}

_bor_inline int borLSetCmp(const bor_lset_t *s1, const bor_lset_t *s2)
{
    int cmp;
    cmp = memcmp(s1->s, s2->s,
                 sizeof(long) * (s1->size < s2->size ?  s1->size : s2->size));
    if (cmp == 0)
        return s1->size - s2->size;
    return cmp;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_LSET_H__ */
