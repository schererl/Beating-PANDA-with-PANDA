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

#ifndef __BOR_IBUCKETQ_H__
#define __BOR_IBUCKETQ_H__

#include <boruvka/compiler.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Number of buckets available in bucket-based queue.
 * Inserting key greater or equal then this constant will end up in program
 * termination.
 */
#define BOR_IBUCKETQ_SIZE 1024

/**
 * Initial size of one bucket in bucket-queue.
 */
#define BOR_IBUCKETQ_BUCKET_INIT_SIZE 32

/**
 * Expansion factor of a bucket.
 */
#define BOR_IBUCKETQ_BUCKET_EXPANSION_FACTOR 2

/**
 * Bucket for storing values.
 */
struct bor_ibucketq_bucket {
    int *value; /*!< Stored values */
    int size;   /*!< Number of stored values */
    int alloc;  /*!< Size of the allocated array */
};
typedef struct bor_ibucketq_bucket bor_ibucketq_bucket_t;

/**
 * Bucket based priority queue.
 */
struct bor_ibucketq {
    bor_ibucketq_bucket_t *bucket; /*!< Array of buckets */
    int bucket_size;              /*!< Number of buckets */
    int lowest_key;               /*!< Lowest key so far */
    int size;                     /*!< Number of elements stored in queue */
};
typedef struct bor_ibucketq bor_ibucketq_t;

/**
 * Initializes priority queue.
 */
void borIBucketQInit(bor_ibucketq_t *pq);

/**
 * Frees allocated resources.
 */
void borIBucketQFree(bor_ibucketq_t *pq);

/**
 * Inserts an element into queue.
 * If the key >= BOR_BUCKET_SIZE the program terminates.
 */
void borIBucketQPush(bor_ibucketq_t *pq, int key, int value);

/**
 * Removes and returns the lowest element.
 * If the queue is empty the program terminates.
 */
int borIBucketQPop(bor_ibucketq_t *pq, int *key);

/**
 * Returns true if the queue is empty.
 */
_bor_inline int borIBucketQIsEmpty(const bor_ibucketq_t *pq);

/**** INLINES ****/
_bor_inline int borIBucketQIsEmpty(const bor_ibucketq_t *pq)
{
    return pq->size == 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_IBUCKETQ_H__ */
