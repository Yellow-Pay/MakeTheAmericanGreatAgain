#ifndef __CONNECTION_H
#define __CONNECTION_H

struct Connection {
    uint32_t src;
    uint32_t dst;
    RingBuffer_t *readRB;
    RingBuffer_t *writeRB;
    Connection(uint32_t src_port, uint32_t dst_port);
    ~Connection();
    int read(int len, char *output); { return rb_read(readRB, len, output); }
    int write(int len, char *input); { return rb_write(writeRB, len, input); }
};

#endif
