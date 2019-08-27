#ifndef __SHM_RINGBUFFER_H
#define __SHM_RINGBUFFER_H
#include <sys/shm.h>

//  ----------------------------------------------------------------
//  |<-    4    ->|<-    4    ->|<-    4    ->|<- SHM_DATA_SIZE -> |
//  ----------------------------------------------------------------
//  |<- headptr ->|<- tailptr ->|<- oldtail ->|<-      DATA     -> |
//  ----------------------------------------------------------------

#define SHM_SIZE 4096
#define METADATA_SIZE 3
#define SHM_DATA_SIZE (SHM_SIZE - METADATA_SIZE * sizeof(uint32_t))
#define GET_HEAD(rb) ((uint32_t *)(rb->address))[0]
#define GET_TAIL(rb) ((uint32_t *)(rb->address))[1]
#define GET_OLDTAIL(rb) ((uint32_t *)(rb->address))[2]

typedef struct RingBuffer {
	int shmid;
	int index;
	char *address;
	char *content;
} RingBuffer_t;

key_t get_idx(int srcPort, int dstPort);
RingBuffer_t* rb_get(key_t k);

RingBuffer_t *rb_init(int key);
void rb_destroy(RingBuffer_t *rb);
int rb_read(RingBuffer_t *rb, int len, char *output);
int rb_write(RingBuffer_t *rb, int len, char *input);

#endif
