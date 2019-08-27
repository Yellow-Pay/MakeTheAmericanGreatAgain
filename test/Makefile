CC=gcc
CXX=g++
test-o=micro-shm micro-socket

test-c=$(test-o:=.c)

all: $(test-o)

run-test: $(test-o)
	./run-test.sh

%: %.c
	$(CC) $^ -o $@
	@echo "./"$@ >> run-test.sh

clean:
	rm -f $(test-o)
	@cat /dev/null > run-test.sh
