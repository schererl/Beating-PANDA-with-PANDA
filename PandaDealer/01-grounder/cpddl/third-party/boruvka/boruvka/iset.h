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

#ifndef __BOR_ISET_H__
#define __BOR_ISET_H__

#include <string.h>
#include <boruvka/compiler.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Array-based set.
 * The elements in .s are always sorted.
 */
struct bor_iset {
    int *s;
    int size;
    int alloc;
};
typedef struct bor_iset bor_iset_t;

#define BOR_ISET_INIT { NULL, 0, 0 }
#define BOR_ISET(NAME) bor_iset_t NAME = BOR_ISET_INIT

#define BOR_ISET_FOR_EACH(S, V) \
    for (int __i = 0; __i < (S)->size && ((V) = (S)->s[__i], 1); ++__i)

#define BOR_ISET_ADD(S, ...) \
    do { \
        int ___bor_iset_vals[] = {__VA_ARGS__}; \
        int ___bor_iset_size = sizeof(___bor_iset_vals) / sizeof(int); \
        for (int i = 0; i < ___bor_iset_size; ++i) \
            borISetAdd((S), ___bor_iset_vals[i]); \
    }while (0)

#define BOR_ISET_SET(S, ...) \
    do { \
        borISetEmpty(S); \
        BOR_ISET_ADD((S), __VA_ARGS__); \
    } while (0)

/**
 * Initialize the set.
 */
void borISetInit(bor_iset_t *s);

/**
 * Frees allocated memory.
 */
void borISetFree(bor_iset_t *s);

/**
 * Returns ith element from the set.
 */
_bor_inline int borISetGet(const bor_iset_t *s, int i);

/**
 * Returns size of the set.
 */
_bor_inline int borISetSize(const bor_iset_t *s);


/**
 * Returns true if val \in s
 */
int borISetHas(const bor_iset_t *s, int val);
_bor_inline int borISetIn(int val, const bor_iset_t *s);

/**
 * Return true if s1 \subset s2
 */
int borISetIsSubset(const bor_iset_t *s1, const bor_iset_t *s2);

/**
 * Returns size of s1 \cap s2.
 */
int borISetIntersectionSize(const bor_iset_t *s1, const bor_iset_t *s2);

/**
 * Returns true if | s1 \cap s2 | >= limit
 */
int borISetIntersectionSizeAtLeast(const bor_iset_t *s1, const bor_iset_t *s2,
                                  int limit);

/**
 * Returns true if | s1 \cap s2 \cap s3 | >= limit
 */
int borISetIntersectionSizeAtLeast3(const bor_iset_t *s1,
                                   const bor_iset_t *s2,
                                   const bor_iset_t *s3,
                                   int limit);

/**
 * Returns true if the sets are disjunct.
 */
_bor_inline int borISetIsDisjunct(const bor_iset_t *s1, const bor_iset_t *s2);

/**
 * s = \emptyset
 */
_bor_inline void borISetEmpty(bor_iset_t *s);

/**
 * d = s
 */
void borISetSet(bor_iset_t *d, const bor_iset_t *s);

/**
 * s = s \cup {val}
 */
void borISetAdd(bor_iset_t *s, int val);

/**
 * s = s \setminus {val}
 * Returns true if val was found in s.
 */
int borISetRm(bor_iset_t *s, int val);

/**
 * dst = dst \cup src
 */
void borISetUnion(bor_iset_t *dst, const bor_iset_t *src);

/**
 * dst = s1 \cup s2
 */
void borISetUnion2(bor_iset_t *dst, const bor_iset_t *s1, const bor_iset_t *s2);

/**
 * dst = dst \cap src
 */
void borISetIntersect(bor_iset_t *dst, const bor_iset_t *src);

/**
 * dst = s1 \cap s2
 */
void borISetIntersect2(bor_iset_t *dst, const bor_iset_t *s1, const bor_iset_t *s2);

/**
 * s1 = s1 \setminus s2
 */
void borISetMinus(bor_iset_t *s1, const bor_iset_t *s2);

/**
 * d = s1 \setminus s2
 */
void borISetMinus2(bor_iset_t *d, const bor_iset_t *s1, const bor_iset_t *s2);


/**
 * Returns true if the sets are equal.
 */
_bor_inline int borISetEq(const bor_iset_t *s1, const bor_iset_t *s2);

/**
 * Compares sets, return values are the same as by memcmp().
 */
_bor_inline int borISetCmp(const bor_iset_t *s1, const bor_iset_t *s2);

/**
 * Remaps the elements of the set using remap array containing maping from
 * the old value to the new value. The mapping must be monotonically
 * increasing and it is assumed that the values in the set are >= 0.
 */
void borISetRemap(bor_iset_t *s, const int *remap);



/**** INLINES: ****/
_bor_inline int borISetGet(const bor_iset_t *s, int i)
{
    return s->s[i];
}

_bor_inline int borISetSize(const bor_iset_t *s)
{
    return s->size;
}

_bor_inline int borISetIn(int val, const bor_iset_t *s)
{
    return borISetHas(s, val);
}

_bor_inline int borISetIsDisjunct(const bor_iset_t *s1, const bor_iset_t *s2)
{
    return !borISetIntersectionSizeAtLeast(s1, s2, 1);
}

_bor_inline void borISetEmpty(bor_iset_t *s)
{
    s->size = 0;
}

_bor_inline int borISetEq(const bor_iset_t *s1, const bor_iset_t *s2)
{
    return s1->size == s2->size
            && memcmp(s1->s, s2->s, sizeof(int) * s1->size) == 0;
}

_bor_inline int borISetCmp(const bor_iset_t *s1, const bor_iset_t *s2)
{
    int cmp;
    cmp = memcmp(s1->s, s2->s,
                 sizeof(int) * (s1->size < s2->size ?  s1->size : s2->size));
    if (cmp == 0)
        return s1->size - s2->size;
    return cmp;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_ISET_H__ */
