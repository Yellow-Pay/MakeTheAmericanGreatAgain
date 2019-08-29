CC=gcc
CXX=g++
CFLAGS=-fPIC -O2 -DNDEBUG 

all: libshm.so
	@ipcrm -a

obj-cpp=RingBuffer.cpp Pool.cpp
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
