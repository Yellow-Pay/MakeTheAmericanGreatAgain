CC=gcc
CXX=g++
obj-c: RingBuffer.cpp

obj-o: $(obj-c:.cpp:.o)

%.o: %.cpp
	$(CXX) -c $^ -o $@

clean:
	rm -f $(obj-o)
