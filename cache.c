#include "cache.h"
#include "csapp.h"

#define MAX_OBJECT_SIZE 102400
#define MAX_CACHE_SIZE 1049000
// Cache helper function
// Initial header, tailer, size of cachelist
void init_cache()
{
	if (header != NULL)
		header = NULL;
	if (tailer != NULL)
		tailer = NULL;
	if (cachesize != 0)
		cachesize = 0;
	//printf("Init_Cache\n");
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

// check a block can add to cache or not
int update_cache(char *uri, char *head, char *body, int headsize, int bodysize) {
	struct Block *newblock;
	newblock = (struct Block *)malloc(sizeof(struct Block));

	if (uri == NULL) {
		printf("Error newblock. Cannot add.\n");
		free(newblock);
		return -1;
	}

	// larger than MAX_OBJECT_SIZE, do nothing
	if (bodysize > MAX_OBJECT_SIZE) {
		free(newblock);
		return 0;
	}

	newblock->body = malloc(strlen(body)*sizeof(char));
	newblock->head = malloc(strlen(head)*sizeof(char));
	newblock->uri = malloc(strlen(uri)*sizeof(char));
	strcpy(newblock->body, body);
	strcpy(newblock->head, head);
	strcpy(newblock->uri, uri);
	newblock->blocksize = bodysize;




	add_block(newblock);

	while (cachesize > MAX_CACHE_SIZE) {
		remove_block(header);
	}

	return 1;
}

// Add a block to tail of cachelist
void add_block(struct Block *newblock) {
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
	cachesize += newblock->blocksize;

	return;
}

// Remove a block from cachelist
void remove_block(struct Block *block) {
	if (block == NULL || header == NULL) {
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
	cachesize -= block->blocksize;

	Free(block->body);
	Free(block->head);
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
	cachesize -= block->blocksize;

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

	printf("header != NULL\n");

	for (search = tailer; search != NULL; search = search->next) {
		if (strcmp(search->uri, uri) == 0)
			return search;
	}

	return NULL;
}

void print_cache(){
	struct Block *itr = tailer;
	if (itr == NULL)
	{
		printf("Cache is empty!!!!!!!!!\n");
	}

	int idx = 0;
	while (itr != NULL){

		printf("%d:\nuri: %s\nsize: %d\n\n",idx,itr-> uri, itr->blocksize);
		itr = itr->next;
		idx++;
	}
	return;
}