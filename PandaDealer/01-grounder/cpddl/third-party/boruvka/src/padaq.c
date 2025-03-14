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

#include "boruvka/alloc.h"
#include "boruvka/padaq.h"

struct _heap_node_t {
    int key;
    void * value;
    bor_pairheap_node_t node;
};
typedef struct _heap_node_t heap_node_t;

static int heapLT(const bor_pairheap_node_t *_n1,
                  const bor_pairheap_node_t *_n2, void *_)
{
    heap_node_t *n1 = bor_container_of(_n1, heap_node_t, node);
    heap_node_t *n2 = bor_container_of(_n2, heap_node_t, node);
    return n1->key <= n2->key;
}

static void heapClear(bor_pairheap_node_t *_n, void *_)
{
    heap_node_t *n = bor_container_of(_n, heap_node_t, node);
    BOR_FREE(n);
}

void borPAdaQInit(bor_padaq_t *pq)
{
    pq->size = 0;
    pq->heap = NULL;
    borPBucketQInit(&pq->bucket);
}

void borPAdaQFree(bor_padaq_t *pq)
{
    borPBucketQFree(&pq->bucket);
    if (pq->heap != NULL){
        borPairHeapClear(pq->heap, heapClear, NULL);
        borPairHeapDel(pq->heap);
    }
}

static void pushHeap(bor_pairheap_t *h, int key, void * value)
{
    heap_node_t *n;

    n = BOR_ALLOC(heap_node_t);
    n->value = value;
    n->key   = key;
    borPairHeapAdd(h, &n->node);
}

static void * popHeap(bor_pairheap_t *h, int *key)
{
    bor_pairheap_node_t *hn;
    heap_node_t *n;
    void * value;

    hn = borPairHeapExtractMin(h);
    n = bor_container_of(hn, heap_node_t, node);
    *key = n->key;
    value = n->value;
    BOR_FREE(n);

    return value;
}

static void bucketToHeap(bor_pbucketq_t *b,
                         bor_pairheap_t *h)
{
    void * val;
    int key;

    while (!borPBucketQIsEmpty(b)){
        val = borPBucketQPop(b, &key);
        pushHeap(h, key, val);
    }
}

void borPAdaQPush(bor_padaq_t *pq, int key, void * value)
{
    if (pq->heap == NULL && (key >= BOR_PBUCKETQ_SIZE || key < 0)){
        pq->heap = borPairHeapNew(heapLT, NULL);
        bucketToHeap(&pq->bucket, pq->heap);
    }

    if (pq->heap != NULL){
        pushHeap(pq->heap, key, value);
    }else{
        borPBucketQPush(&pq->bucket, key, value);
    }
    ++pq->size;
}

void * borPAdaQPop(bor_padaq_t *pq, int *key)
{
    if (pq->heap != NULL){
        --pq->size;
        return popHeap(pq->heap, key);
    }else{
        --pq->size;
        return borPBucketQPop(&pq->bucket, key);
    }
}
