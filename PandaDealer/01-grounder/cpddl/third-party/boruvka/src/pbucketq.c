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

#include <stdio.h>
#include "boruvka/alloc.h"
#include "boruvka/pbucketq.h"

void borPBucketQInit(bor_pbucketq_t *q)
{
    q->bucket_size = BOR_PBUCKETQ_SIZE;
    q->bucket = BOR_CALLOC_ARR(bor_pbucketq_bucket_t, q->bucket_size);
    q->lowest_key = q->bucket_size;
    q->size = 0;
}

void borPBucketQFree(bor_pbucketq_t *q)
{
    int i;

    for (i = 0; i < q->bucket_size; ++i){
        if (q->bucket[i].value != NULL)
            BOR_FREE(q->bucket[i].value);
    }
    if (q->bucket != NULL)
        BOR_FREE(q->bucket);
}

void borPBucketQPush(bor_pbucketq_t *q, int key, void * value)
{
    bor_pbucketq_bucket_t *bucket;

    if (key >= q->bucket_size){
        fprintf(stderr, "Error: borPBucketQ: key %d is over a size of"
                        " the bucket queue, which is %d.\n",
                        key, BOR_PBUCKETQ_SIZE);
        exit(-1);
    }

    bucket = q->bucket + key;
    if (bucket->value == NULL){
        bucket->alloc = BOR_PBUCKETQ_BUCKET_INIT_SIZE;
        bucket->value = BOR_ALLOC_ARR(void *, bucket->alloc);

    }else if (bucket->size >= bucket->alloc){
        bucket->alloc *= BOR_PBUCKETQ_BUCKET_EXPANSION_FACTOR;
        bucket->value = BOR_REALLOC_ARR(bucket->value,
                                        void *, bucket->alloc);
    }
    bucket->value[bucket->size++] = value;
    ++q->size;

    if (key < q->lowest_key)
        q->lowest_key = key;
}

void * borPBucketQPop(bor_pbucketq_t *q, int *key)
{
    bor_pbucketq_bucket_t *bucket;
    void * val;

    if (q->size == 0){
        fprintf(stderr, "Error: borPBucketQ is empty!\n");
        exit(-1);
    }

    bucket = q->bucket + q->lowest_key;
    while (bucket->size == 0){
        ++q->lowest_key;
        bucket += 1;
    }

    val = bucket->value[--bucket->size];
    *key = q->lowest_key;
    --q->size;
    return val;
}
