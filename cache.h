#include "csapp.h"

struct Block *header;
struct Block *tailer;
int cachesize;
/*structure for caching items from sever
*/
struct Block
{
	char *uri;
	char *head;
	char *body;
	struct Block *prev;
	struct Block *next;
	int blocksize;
};

/*struct Cache_List
{
	struct Block *header;
	struct Block *tailer;
	int cachesize;
};*/

// Fuction Prototype
void init_cache();
//struct block *init_block(size_t blocksize);
void add_block(struct Block *block);
void remove_block(struct Block *block);
void move_to_tail(struct Block *block);
struct Block *find_block(char *uri);
//char *find_body(char *uri);
int update_cache(char *uri, char *head, char *body, int headsize, int bodysize);

void print_cache();