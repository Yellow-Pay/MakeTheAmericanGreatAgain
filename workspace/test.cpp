#include <iostream>
#include <cassert>
#include <set>
#include <map>
#include <pthread.h>
#include <string.h>
using namespace std;
#define NUM_THREADS 10
pthread_t tids[NUM_THREADS];
map<int, bool> send_set;

void init() {
	for (int i = 0; i < NUM_THREADS; i++) {
		for (int j = 0; j < NUM_THREADS; j++) {
			if (i == j) continue;
			send_set[i * NUM_THREADS + j] = false;
		}
	}
}

int send(int src, int dest, char *buf, int size) {
	send_set[src * NUM_THREADS + dest] = true;
	return 0;
}
int recv(int src, int dest, char *buf, int size) {
	if (!send_set[src * NUM_THREADS + dest]) {
		return 0;
	}
	int ret = sprintf(buf, "src:%04d, dest:%04d\n", src, dest);
	return ret;
}

void *ping_pong(void *arg) {
	int idx = *(int *)arg;
	for (int i = 0; i < NUM_THREADS; i++) {
		if (i != idx) {
			char buf[1024];
			sprintf(buf, "src:%04d, dest:%04d\n", idx, i);	
			//cout << "Send - " << buf << endl;
			send(idx, i, buf, 1024);  
		}
	}
	for (int i = 0; i < NUM_THREADS; i++) {
		if (i != idx) {
			char buf[1024];
			char ans[1024];
			sprintf(ans, "src:%04d, dest:%04d\n", i, idx);	
			memset(buf, 0, 1024);
			while (!recv(i, idx, buf, 1024)) {
				;
			}
		//	cout << "BUF = " << buf << endl;
		//	cout << "Ans = " << ans << endl;
			assert(!strcmp(buf, ans));
		}
	}
}

int main(int argc, char *argv[]) {
	init();
	for (int i = 0; i < NUM_THREADS; i++) {
		int *arg = (int *)malloc(sizeof(*arg));
		*arg = i;
		int ret = pthread_create(&tids[i], NULL, ping_pong, arg);
		if (ret) {
			cerr << "pthread_create error: error_code = " << ret << endl;
		}
	}
	for (int i = 0; i < NUM_THREADS; i++) {
		pthread_join(tids[i], NULL);
	}
	return 0;
}
