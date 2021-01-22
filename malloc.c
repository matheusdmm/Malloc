#include <assert.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>


#define META_SIZE sizeof(struct block_meta)

void *global_base = NULL;

//	prototype for each memmory block
struct block_meta {
	size_t size;
	struct block_meta *next;
	int dr_doom;	// For debug
	int free;
};

struct block_meta *find_free_block(struct block_meta **last, size_t size) {
	struct block_meta *current = global_base;
	while (current && !(current -> free && current -> size >= size)) {
		*last = current;
		current = current -> next;
	};
}

//	Request space from the OS using sbrk to add our new block to the end of an linked list
struct block_meta *request_space(struct block_meta* last, size_t size) {
	struct block_meta *block;
	block = sbrk(0);
	void *request = sbrk(size + META_SIZE);
	assert ((void*)block == request);

	if (request == (void*) - 1) {
		return NULL;
	}

	if (last) {
		last -> next = block;
	}

	block -> size = size;
	block -> next = NULL;
	block -> free = 0x12345678;
	return block;
}

void *realloc(void *ptr, size_t size) {
	if (!ptr) {
		return malloc(size);
	}
	
	struct block_meta* block_ptr = get_block_ptr(ptr);
	if (block_ptr -> size >= size) {
		return ptr;
	}

	//	Realloc, Malloc new space and free the old space
	//	Copies old data to new space
	void *new_ptr;
	new_ptr = malloc(size);
	if (!new_ptr) {
		return NULL;
	}
	memcpy(new_ptr, ptr, block_ptr -> size);
	free(ptr);
	return new_ptr;
}

void *malloc(size_t size) {
  struct block_meta *block;
  if (size <= 0) {
    return NULL;
  }

  if (!global_base) {
    block = request_space(NULL, size);
    if (!block) {
      return NULL;
    }
    global_base = block;
  } else {
    struct block_meta *last = global_base;
    block = find_free_block(&last, size);
    if (!block) {
      block = request_space(last, size);
      if (!block) {
        return NULL;
      }
    } else {
      block -> free = 0;
      block -> dr_doom = 0x77777777;
    }
  }
  return(block + 1);
}

void *calloc(size_t nelem, size_t elsize) {
	size_t size = nelem * elsize;
	void *ptr = malloc(size);
	memset(ptr, 0, size);
	return ptr;
}

void free(void *ptr) {
	if (!ptr) {
		return;
	}

	struct block_meta* block_ptr = get_block_ptr(ptr);
	assert(block_ptr -> free == 0);
	assert(block_ptr -> dr_doom == 0x77777777 || block_ptr -> dr_doom == 0x12345678);
	block_ptr -> free = 1;
	block_ptr -> dr_doom = 0x55555555;
}