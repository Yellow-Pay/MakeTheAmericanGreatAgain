CC=gcc
CXX=g++
obj-c=RingBuffer.cpp Connection.cpp

obj-o=$(obj-c:.cpp=.o)

all: $(obj-o)

%.o: %.cpp
	$(CXX) -c $^ -o $@

clean:
	rm -f $(obj-o)
