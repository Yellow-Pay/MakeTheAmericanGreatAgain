UNAME := $(shell uname)
CC=gcc
ifeq ($(UNAME),Linux)
CXX=g++
endif
ifeq ($(UNAME),Darwin)
CXX=g++ -std=c++11 -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.14.sdk
endif
obj-c=RingBuffer.cpp Connection.cpp Pool.cpp

obj-o=$(obj-c:.cpp=.o)

all: $(obj-o)

test: TestConcurrency.cpp $(obj-o)
	$(CXX) $^ -o $@

%.o: %.cpp
	$(CXX) -c $^ -o $@

clean:
	rm -f $(obj-o) test
