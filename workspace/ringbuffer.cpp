#include <iostream>
#include <cstring>
using namespace std;
typedef struct CircularBuffer {
	// beginIdx == endIdx -> empty
	// beginIdx == (endIdx + 1) % size -> full
	uint64_t beginIdx;
	uint64_t endIdx;
	uint64_t size;
	char *data;
} CircularBuffer_t;

CircularBuffer_t *createCircularBuffer(uint64_t size) {
	CircularBuffer_t *buffer = (CircularBuffer_t *)malloc(sizeof(CircularBuffer_t));
	if (!buffer) {
		return NULL;
	}
	buffer->data = (char *)malloc(size);
	if (!buffer->data) {
		free(buffer);
		return NULL;
	}
	buffer->beginIdx = 0;
	buffer->endIdx = 0;
	buffer->size = size;
	return buffer;
}

void destroyCircularBuffer(CircularBuffer_t *buffer) {
	free(buffer->data);
	free(buffer);
}

int read(CircularBuffer_t *buffer, int len, char *output) {
	uint64_t begin = buffer->beginIdx;
	uint64_t end = buffer->endIdx;
	uint64_t size = buffer->size;
	char *data = buffer->data;
	uint64_t readSize = 0;
	while (begin != end && readSize != len) {
		output[readSize] = data[begin];
		readSize++;
		begin = (begin + 1) % size;
	}
	buffer->beginIdx = begin;
	cout << "[Read] len = " << len << ", "
		<< "begin = " << buffer->beginIdx << ", "
		<< "end = " << buffer->endIdx << endl;
	return readSize;
}

int write(CircularBuffer_t *buffer, int len, char *input) {
	uint64_t begin = buffer->beginIdx;
	uint64_t end = buffer->endIdx;
	uint64_t size = buffer->size;
	char *data = buffer->data;
	uint64_t writtenSize = 0;
	while ((end + 1) % size != begin && writtenSize != len) {
		data[end] = input[writtenSize];
		writtenSize++;
		end = (end + 1) % size;
	}
	buffer->endIdx = end;
	cout << "[Write] len = " << len << ", "
		<< "begin = " << buffer->beginIdx << ", "
		<< "end = " << buffer->endIdx << endl;
	return writtenSize;
}

void testRW(CircularBuffer_t *buffer, char *str, bool isWrite) {
	int retval = 0;
	if (isWrite) {
		retval = write(buffer, strlen(str), str);
		cout << "retval = " << retval << endl;
	} else {
		retval = read(buffer, 128, str);
		cout << "retval = " << retval << endl;
		cout << "content = " << str << endl;
		memset(str, 0, strlen(str));
	}
}

char s1[] = "Hello world";
char s2[] = "Merry chrismas!";
char s3[] = "Make the American great again!";

void test1(CircularBuffer_t *buffer) {
	char *tmp = (char *)malloc(128);
	testRW(buffer, s1, true);
	testRW(buffer, tmp, false);
	testRW(buffer, s2, true);
	testRW(buffer, tmp, false);
	testRW(buffer, s3, true);
	testRW(buffer, tmp, false);
	free(tmp);
	cout << endl;
}

void test2(CircularBuffer_t *buffer) {
	char *tmp = (char *)malloc(128);
	testRW(buffer, s1, true);
	testRW(buffer, s2, true);
	testRW(buffer, tmp, false);
	testRW(buffer, tmp, false);
	testRW(buffer, s3, true);
	testRW(buffer, tmp, false);
	free(tmp);
	cout << endl;
}

int main() {
	CircularBuffer_t *buffer = createCircularBuffer(1024);
	test1(buffer);
	test2(buffer);
	destroyCircularBuffer(buffer);
	return 0;
}
