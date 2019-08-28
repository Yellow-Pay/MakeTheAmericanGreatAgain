#include <memory>
#include <sys/shm.h>

const int POOL_SHM_KEY = 0x3f3f3f3f;
const int META_FILED_SIZE = 2;
using index_type = uint32_t;
const int CHANNEL_SIZE = 4096 - META_FILED_SIZE;

/*
   | dummy head | content | next channel key |
   |  sizeof(index_type) bytes  | pool_size * sizeof(index_type) |
   sizeof(index_type) bytes |
   */
struct Channel {
	index_type *address = nullptr;
	std::shared_ptr<Channel> next = nullptr;
	int next_channel_index = CHANNEL_SIZE + 1;
	key_t key;
	int size = CHANNEL_SIZE;
	int shmid;
	int get_channel_shm_size() {
		return (CHANNEL_SIZE + META_FILED_SIZE) * sizeof(index_type);
	}
	Channel(key_t k) {
		shmid = shmget(k, get_channel_shm_size(), 0666 | IPC_CREAT);
		shmid_ds info;
		shmctl(shmid, IPC_STAT, &info);
		address = (index_type *)shmat(shmid, (void *)0, 0);
		if (info.shm_nattch == 0) {
			// global init, accross processes
			for (int i = 0; i <= size; ++i) {
				address[i] = i + 1;
			}
			address[CHANNEL_SIZE] = 0;
			address[next_channel_index] = 0;
		} else {
			if (address[next_channel_index] != 0) {
				next = std::make_shared<Channel>(address[next_channel_index]);
			}
		}
	}
	~Channel() {
		shmdt(address);
		shmid_ds info;
		shmctl(shmid, IPC_STAT, &info);
		if (info.shm_nattch == 0) {
			shmctl(shmid, IPC_RMID, NULL);
		}
	}
	bool isFull() { return address[0] == 0; }
	void allocNextChannel() {
		next = std::make_shared<Channel>(key + 1);
		address[next_channel_index] = key + 1;
	}
	int get() {
		int ret, new_next;
retry:
		ret = address[0];
		new_next = address[ret];
		// all-or-nothing op
		if (!__sync_bool_compare_and_swap(&address[0], ret, new_next)) {
			goto retry;
		}
		return ret;
	}
	void release(int index) {
		int old_next, next;
retry:
		old_next = address[0];	
		address[index] = old_next;
		// all-or-nothing op
		if (!__sync_bool_compare_and_swap(&address[0], old_next, index)) {
			goto retry;
		}
	}
};

struct Pool {
	std::shared_ptr<Channel> head;
	Pool() { head = std::make_shared<Channel>(POOL_SHM_KEY); }
	int get() {
		auto current = head;
		int prefix = 0;
		while (true) {
			int r = current->get();
			if (r > 0) {
				return prefix + r;
			}
			prefix += current->size;
			if (!current->next) {
				current->allocNextChannel();
			}
			current = current->next;
		}
		return 0;
	}
	void release(int index) {
		auto current = head;
		while (index > current->size) {
			index -= current->size;
			current = current->next;
		}
		current->release(index);
	}
};

Pool p;
// get a free ringbuffer, make it busy, return its index
// if index <= 0, get fails.
int pool_get() { return p.get(); }
// release a ringbuffer, make it idle
void pool_release(int index) { p.release(index); }
