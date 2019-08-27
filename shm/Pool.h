#ifndef __POOL_H
#define __POOL_H

int pool_get(); // get a free ringbuffer, make it busy, return its index
void pool_release(int index);    // release a ringbuffer, make it idle

#endif // __POOL_H