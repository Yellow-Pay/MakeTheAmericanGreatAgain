UNAME := $(shell uname)
CC=gcc
ifeq ($(UNAME),Linux)
CXX=g++
endif
ifeq ($(UNAME),Darwin)
CXX=g++ -g -std=c++11 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk
endif

CFLAGS=-fPIC -O2 -DNDEBUG 

all: libshm.so test

client: client.c
	$(CC) $^ -o $@

server: server.c
	$(CC) $^ -o $@

TestConcurrencyRead: TestConcurrencyRead.cpp
	$(CXX) $^ -o $@ -lpthread

TestConcurrencyWrite: TestConcurrencyWrite.cpp
	$(CXX) $^ -o $@ -lpthread

test-bin=client server TestConcurrencyWrite TestConcurrencyRead
test: $(test-bin)

obj-cpp=RingBuffer.cpp Connection.cpp Pool.cpp
obj-c=socket.c

obj-o=$(obj-cpp:.cpp=.o) $(obj-c:.c=.o)

libshm.so: $(obj-o)
	$(CXX) $^ -lstdc++ -fPIC -shared -o $@

%.o: %.cpp
	$(CXX) -c $(CFLAGS) $^ -o $@

%.o: %.c
	$(CXX) -c $(CFLAGS) $^ -o $@
	
clean:
	rm -f $(obj-o) $(test-bin) libshm.so
