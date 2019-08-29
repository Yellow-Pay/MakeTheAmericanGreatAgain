#ifndef __SHM_RINGBUFFER_H
#define __SHM_RINGBUFFER_H
#include <sys/shm.h>

//  ----------------------------------------------------------------
//  |<-    4    ->|<-    4    ->|<-    4    ->|<- SHM_DATA_SIZE -> |
//  ----------------------------------------------------------------
//  |<- headptr ->|<- tailptr ->|<- oldtail ->|<-      DATA     -> |
//  ----------------------------------------------------------------

#define SHM_SIZE 4096 * 512
#define METADATA_SIZE 3
#define SHM_DATA_SIZE (SHM_SIZE - METADATA_SIZE * sizeof(uint32_t))
#define GET_HEAD(rb) ((uint32_t *)(rb->address))[0]
#define GET_TAIL(rb) ((uint32_t *)(rb->address))[1]
#define GET_OLDTAIL(rb) ((uint32_t *)(rb->address))[2]

/*
The basic data struct of our shm-based ipc.
 ----------------------------------------------------------------
 |<-    4    ->|<-    4    ->|<-    4    ->|<- SHM_DATA_SIZE -> |
 ----------------------------------------------------------------
 |<- headptr ->|<- tailptr ->|<- oldtail ->|<-      DATA     -> |
 ----------------------------------------------------------------
*/
typedef struct RingBuffer {
    int shmid;     /* shmid from shmget */
    int index;     /* the index of RingBuffer in Pool */
    char *address; /* the memory address of RingBuffer in local process memory
                      space */
    char *content; /* the address of content of RingBuffer, skips the metadata
                      (three pointers) */
} RingBuffer_t;

/*
get the index of RingBuffer according to srcPort and dstPort.
1. get the index_table according to srcPort from shared memory
2. find the index from index_table[dstPort]
*/
key_t get_idx(int srcPort, int dstPort);
/*
set the index of RingBuffer to 0 according to srcPort and dstPort
*/
void clear_idx(int srcPort, int dstPort);

/*
All functions to manipulate RingBuffer
*/
RingBuffer_t *rb_get(key_t k);
RingBuffer_t *rb_init(int key);
void rb_destroy(RingBuffer_t *rb);
int rb_read(RingBuffer_t *rb, int len, char *output);
int rb_write(RingBuffer_t *rb, int len, char *input);

#endif
