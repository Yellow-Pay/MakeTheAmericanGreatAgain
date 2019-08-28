UNAME := $(shell uname)
CC=gcc
ifeq ($(UNAME),Linux)
CXX=g++
endif
ifeq ($(UNAME),Darwin)
CXX=g++ -g -std=c++11 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk
endif

CFLAGS=-fPIC -O2

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
	rm -f $(obj-o) libshm.so
