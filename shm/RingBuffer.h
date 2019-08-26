#ifndef __SHM_RINGBUFFER_H
#define __SHM_RINGBUFFER_H
#include <sys/shm.h>

//  ----------------------------------------------------------------
//  |<-    4    ->|<-    4    ->|<-    4    ->|<- SHM_DATA_SIZE -> |
//  ----------------------------------------------------------------
//  |<- headptr ->|<- tailptr ->|<- oldtail ->|<-      DATA     -> |
//  ----------------------------------------------------------------

#define SHM_SIZE 32
#define SHM_DATA_SIZE (SHM_SIZE - 2 * sizeof(uint32_t))
#define GET_HEAD(rb) ((uint32_t *)(rb->address))[0]
#define GET_TAIL(rb) ((uint32_t *)(rb->address))[1]
#define GET_OLDTAIL(rb) ((uint32_t *)(rb->address))[2]

typedef struct RingBuffer {
	int shmid;
	char *address;
	char *content;
} RingBuffer_t;

RingBuffer_t *rb_init(key_t key);
void rb_destroy(RingBuffer_t *rb);
int read(RingBuffer_t *rb, int len, char *output);
int write(RingBuffer_t *rb, int len, char *input);

#endif
