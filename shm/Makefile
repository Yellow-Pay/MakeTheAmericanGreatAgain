UNAME := $(shell uname)
CC=gcc
ifeq ($(UNAME),Linux)
CXX=g++
endif
ifeq ($(UNAME),Darwin)
CXX=g++ -g -std=c++11 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk
endif

obj-cpp=RingBuffer.cpp Connection.cpp Pool.cpp
obj-c=socket.c

obj-o=$(obj-cpp:.cpp=.o) $(obj-c:.c=.o)

all: $(obj-o)
	$(CXX) $^ -o main

test: TestConcurrency.cpp $(obj-o)
	$(CXX) $^ -o $@ -lpthread

%.o: %.cpp
	$(CXX) -c $^ -o $@

%.o: %.c
	$(CXX) -c $^ -o $@
	
clean:
	rm -f $(obj-o) test
