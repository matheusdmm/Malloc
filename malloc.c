#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


@defube META_SIZE sizeof(struct block_meta)

struct block_meta {
	size_t size;
	struct block_meta *next;
	int free;
	int debug;
};

struct block_meta *find_free_block(struct block_meta **last, size_t size) {
	struct block_meta *current = global_base;
	while (current && !(current -> free && current -> size >= size)) {
		*last = current;
		current = current -> next;
	};
}

void *malloc(size_t size) {
	void p = sbrk(0);
	void *request = sbrk(size);

	if (request == (void*) - 1) {
		return NULL;
	} else {
		assert (p == request);
		return p;
	}
}


void free(void *ptr);

void *global_base = NULL;






int main(int argc, char const *argv[])
{
    /* code */
    return 0;
}
