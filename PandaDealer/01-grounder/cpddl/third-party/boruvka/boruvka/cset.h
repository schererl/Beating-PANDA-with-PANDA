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

#ifndef __BOR_CSET_H__
#define __BOR_CSET_H__

#include <string.h>
#include <boruvka/compiler.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Array-based set.
 * The elements in .s are always sorted.
 */
struct bor_cset {
    char *s;
    int size;
    int alloc;
};
typedef struct bor_cset bor_cset_t;

#define BOR_CSET_INIT { NULL, 0, 0 }
#define BOR_CSET(NAME) bor_cset_t NAME = BOR_CSET_INIT

#define BOR_CSET_FOR_EACH(S, V) \
    for (int __i = 0; __i < (S)->size && ((V) = (S)->s[__i], 1); ++__i)

#define BOR_CSET_ADD(S, ...) \
    do { \
        char ___bor_cset_vals[] = {__VA_ARGS__}; \
        int ___bor_cset_size = sizeof(___bor_cset_vals) / sizeof(char); \
        for (int i = 0; i < ___bor_cset_size; ++i) \
            borCSetAdd((S), ___bor_cset_vals[i]); \
    }while (0)

#define BOR_CSET_SET(S, ...) \
    do { \
        borCSetEmpty(S); \
        BOR_CSET_ADD((S), __VA_ARGS__); \
    } while (0)

/**
 * Initialize the set.
 */
void borCSetInit(bor_cset_t *s);

/**
 * Frees allocated memory.
 */
void borCSetFree(bor_cset_t *s);

/**
 * Returns ith element from the set.
 */
_bor_inline char borCSetGet(const bor_cset_t *s, int i);

/**
 * Returns size of the set.
 */
_bor_inline int borCSetSize(const bor_cset_t *s);


/**
 * Returns true if val \in s
 */
int borCSetHas(const bor_cset_t *s, char val);
_bor_inline int borCSetIn(char val, const bor_cset_t *s);

/**
 * Return true if s1 \subset s2
 */
int borCSetIsSubset(const bor_cset_t *s1, const bor_cset_t *s2);

/**
 * Returns size of s1 \cap s2.
 */
int borCSetIntersectionSize(const bor_cset_t *s1, const bor_cset_t *s2);

/**
 * Returns true if | s1 \cap s2 | >= limit
 */
int borCSetIntersectionSizeAtLeast(const bor_cset_t *s1, const bor_cset_t *s2,
                                  int limit);

/**
 * Returns true if | s1 \cap s2 \cap s3 | >= limit
 */
int borCSetIntersectionSizeAtLeast3(const bor_cset_t *s1,
                                   const bor_cset_t *s2,
                                   const bor_cset_t *s3,
                                   int limit);

/**
 * Returns true if the sets are disjunct.
 */
_bor_inline int borCSetIsDisjunct(const bor_cset_t *s1, const bor_cset_t *s2);

/**
 * s = \emptyset
 */
_bor_inline void borCSetEmpty(bor_cset_t *s);

/**
 * d = s
 */
void borCSetSet(bor_cset_t *d, const bor_cset_t *s);

/**
 * s = s \cup {val}
 */
void borCSetAdd(bor_cset_t *s, char val);

/**
 * s = s \setminus {val}
 * Returns true if val was found in s.
 */
int borCSetRm(bor_cset_t *s, char val);

/**
 * dst = dst \cup src
 */
void borCSetUnion(bor_cset_t *dst, const bor_cset_t *src);

/**
 * dst = s1 \cup s2
 */
void borCSetUnion2(bor_cset_t *dst, const bor_cset_t *s1, const bor_cset_t *s2);

/**
 * dst = dst \cap src
 */
void borCSetIntersect(bor_cset_t *dst, const bor_cset_t *src);

/**
 * dst = s1 \cap s2
 */
void borCSetIntersect2(bor_cset_t *dst, const bor_cset_t *s1, const bor_cset_t *s2);

/**
 * s1 = s1 \setminus s2
 */
void borCSetMinus(bor_cset_t *s1, const bor_cset_t *s2);

/**
 * d = s1 \setminus s2
 */
void borCSetMinus2(bor_cset_t *d, const bor_cset_t *s1, const bor_cset_t *s2);


/**
 * Returns true if the sets are equal.
 */
_bor_inline int borCSetEq(const bor_cset_t *s1, const bor_cset_t *s2);

/**
 * Compares sets, return values are the same as by memcmp().
 */
_bor_inline int borCSetCmp(const bor_cset_t *s1, const bor_cset_t *s2);

/**
 * Remaps the elements of the set using remap array containing maping from
 * the old value to the new value. The mapping must be monotonically
 * increasing and it is assumed that the values in the set are >= 0.
 */
void borCSetRemap(bor_cset_t *s, const char *remap);



/**** INLINES: ****/
_bor_inline char borCSetGet(const bor_cset_t *s, int i)
{
    return s->s[i];
}

_bor_inline int borCSetSize(const bor_cset_t *s)
{
    return s->size;
}

_bor_inline int borCSetIn(char val, const bor_cset_t *s)
{
    return borCSetHas(s, val);
}

_bor_inline int borCSetIsDisjunct(const bor_cset_t *s1, const bor_cset_t *s2)
{
    return !borCSetIntersectionSizeAtLeast(s1, s2, 1);
}

_bor_inline void borCSetEmpty(bor_cset_t *s)
{
    s->size = 0;
}

_bor_inline int borCSetEq(const bor_cset_t *s1, const bor_cset_t *s2)
{
    return s1->size == s2->size
            && memcmp(s1->s, s2->s, sizeof(char) * s1->size) == 0;
}

_bor_inline int borCSetCmp(const bor_cset_t *s1, const bor_cset_t *s2)
{
    int cmp;
    cmp = memcmp(s1->s, s2->s,
                 sizeof(char) * (s1->size < s2->size ?  s1->size : s2->size));
    if (cmp == 0)
        return s1->size - s2->size;
    return cmp;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_CSET_H__ */
