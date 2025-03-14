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

#ifndef __BOR_IADAQ_H__
#define __BOR_IADAQ_H__

#include <boruvka/compiler.h>
#include <boruvka/pairheap.h>
#include <boruvka/ibucketq.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * Adaptive priority queue that can switch between bucket queue and
 * pairheap queue.
 */
struct bor_iadaq {
    bor_ibucketq_t bucket;
    bor_pairheap_t *heap;
    int size;
};
typedef struct bor_iadaq bor_iadaq_t;

/**
 * Initializes priority queue.
 */
void borIAdaQInit(bor_iadaq_t *pq);

/**
 * Frees allocated resources.
 */
void borIAdaQFree(bor_iadaq_t *pq);

/**
 * Inserts an element into queue.
 * If the key >= BOR_ADA_SIZE the program terminates.
 */
void borIAdaQPush(bor_iadaq_t *pq, int key, int value);

/**
 * Removes and returns the lowest element.
 * If the queue is empty the program terminates.
 */
int borIAdaQPop(bor_iadaq_t *pq, int *key);

/**
 * Returns true if the queue is empty.
 */
_bor_inline int borIAdaQIsEmpty(const bor_iadaq_t *pq);

/**** INLINES ****/
_bor_inline int borIAdaQIsEmpty(const bor_iadaq_t *pq)
{
    return pq->size == 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BOR_IADAQ_H__ */

