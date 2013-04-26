#include <csapp.h>

#define MAX_CACHE_SIZE = 1049000
#define MAX_OBJECT_SIZE = 102400

/*structure for caching items from sever
*/
struct Block
{
	char *uri;
	char *body;
	char *prev;
	char *next;
	int bodysize;
};

struct Block *cache_list;

// Fuction Prototype
void init_cache();
//struct block *init_block(size_t bodysize);
void add_block(char *uri, char *body, int bodysize);
void remove_block(struct Block *block);
void move_to_tail(struct Block *block);
struct Block *find_block(char *uri);
//char *find_body(char *uri);


// Cache helper function
// Initial header, tailer, size of cachelist
void init_cache() {
	if (header != NULL)
		header = NULL;
	if (tailer != NULL)
		tailer = NULL;
	if (cachesize != 0)
		cachesize = 0;
	return;
}

// Initial a block
/*struct block *init_block(char *uri, size_t bodysize) {
	struct block *newblock;

	if (bodysize > 0 && uri != NULL) {
		newblock->body = malloc(bodysize);
		newblock->uri = uri;
	}
	else 
		return NULL;	
}*/

// Add a block to tail of cachelist
void add_block(char *uri, char *body, int bodysize) {
	struct block *newblock;

	if (uri == NULL || body == NULL || bodysize < 1) {
		printf("Error newblock. Cannot add.\n");
		return;
	}

	newblock->body = malloc(bodysize);
	newblock->uri = uri;
	newblcok->bodysize = bodysize;

	// if newblock is not NULL, add to tail
	if (header == NULL) {		// cachelist is empty
		header = newblock;
		tailer = newblock;
	}
	else {						// cachelist is not empty
		newblock->next = tailer;
		newblock->prev = NULL;
		tailer->prev = newblock;
		tailer = newblock;
	}

	// Increase cache size
	cachesize += newblock->bodysize;

	return;
}

// Remove a block from cachelist
void remove_block(struct Block *block) {
	if (newblock == NULL || header == NULL) {
		printf("Error remove.\n");
		return;
	}

	/* Remove from list */
	// Case 1. it's first block
	if (block->prev	== NULL) {
		tailer = block->next;
		tailer->prev = NULL;
	}
	// Case 2. it's last block
	else if (block->next == NULL) {
		header = block->prev;
		header->next = NULL;
	}
	// Case 3. it's in middle
	else {
		block->prev->next = block->next;
		block->next->prev = block->prev;
	}

	// Decrease cache size
	cachesize -= block->bodysize;

	Free(block);
	return;
}

// Move recent-read-block to tail, for LRU
void move_to_tail(struct Block *block) {
	if (tailer == NULL || block == NULL) {
		printf("Error moving to first\n");
		return;
	}

	/* Remove block form list */
	// Case 1. Already the first block
	if (block->prev == NULL)
		return;
	// Case 2. it's last block
	if (block->next == NULL) {
		header = block->prev;
		header->next = NULL;
	}
	// Case 3. it's in middle
	else {
		block->prev->next = block->next;
		block->next->prev = block->prev;
	}

	add_block(block);
	cachesize -= block->bodysize;

	return;
}

// Search block by uri
struct Block *find_block(char *uri) {
	struct Block *search;

	if (uri == NULL) {
		printf("Error URI. Cannot search\n");
		return NULL;
	}

	if (header == NULL) {
		printf("Error. Cachelist is empty\n");
		return NULL;
	}

	for (search = tailer; search != NULL; search = search->next) {
		if (strcmp(search->uri, uri) == 0)
			return search;
	}

	return NULL;
}