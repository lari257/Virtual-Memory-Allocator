// COPYRIGHT: Larisa Florea

#include "vma.h"

// allocate a new arena
arena_t *alloc_arena(const uint64_t size)
{
	arena_t *arena = malloc(sizeof(*arena));
	if (!arena)
		fprintf(stderr, "This zone could not be allocated\n");
	arena->arena_size = size;
	arena->alloc_list = NULL;

	return arena;
}

// deallocate an arena
void dealloc_arena(arena_t *arena)
{
	if (!arena->alloc_list)
		return;

	node *curr1, *curr2;
	node *prev1, *prev2;
	curr1 = arena->alloc_list->head;

	while (curr1) {
		list_t *l = (list_t *)curr1->data_b->miniblock_list;
		curr2 = l->head;
		while (curr2) {
			prev2 = curr2;
			curr2 = curr2->next;
			int8_t *buffer = (int8_t *)prev2->data_mb->rw_buffer;
			free(buffer);
			free(prev2->data_mb);
			free(prev2);
		}
		prev1 = curr1;
		curr1 = curr1->next;
		free(l);
		free(prev1->data_b);
		free(prev1);
	}
	free(arena->alloc_list);
}

// allocate a new list
list_t *create_list(void)
{
	list_t *list = malloc(sizeof(*list));
	if (!list)
		fprintf(stderr, "This zone could not be allocated\n");
	list->head = NULL;
	list->size = 0;
	list->list_size = 0;

	return list;
}

// return the n-th node from a list
node *get_nth_node(list_t *list, long n)
{
	if (!list->head)
		return NULL;

	node *curr = list->head;

	while (n > 0) {
		curr = curr->next;
		n--;
	}

	return curr;
}

// add a new node to the n-th position in a list
node *add_nth_node(list_t *list, long n)
{
	node *curr = get_nth_node(list, n - 1);
	node *new_node = malloc(sizeof(*new_node));
	if (!new_node)
		fprintf(stderr, "This zone could not be allocated\n");

	if (list->size == 0) {
		list->head = new_node;
		new_node->next = NULL;
		new_node->prev = NULL;

	} else {
		if ((long)list->size == n) {
			curr->next = new_node;
			new_node->prev = curr;
			new_node->next = NULL;
		} else {
			if (n == 0) {
				new_node->next = curr;
				list->head = new_node;
				new_node->prev = NULL;
				curr->prev = list->head;
			} else {
				new_node->prev = curr;
				new_node->next = curr->next;

				curr->next->prev = new_node;
				curr->next = new_node;
			}
		}
	}

	list->size++;
	return new_node;
}

// add a new miniblock
void add_new_miniblock(node *node, uint64_t address, uint64_t size, long n)
{
	list_t *l = (list_t *)(node->data_b->miniblock_list);

	// add the new node to the n-th position in the list
	struct node *new_node = add_nth_node(l, n);

	// allocate the new miniblock
	new_node->data_mb = malloc(sizeof(miniblock_t));
	if (!new_node->data_mb)
		fprintf(stderr, "This zone could not be allocated\n");

	// initialize the new miniblock
	new_node->data_mb->start_address = address;
	new_node->data_mb->size = size;
	new_node->data_mb->perm = 6;
	new_node->data_mb->rw_buffer = calloc(size, sizeof(int8_t));
	if (!new_node->data_mb->rw_buffer)
		fprintf(stderr, "This zone could not be allocated\n");

	l->list_size += size;
	node->data_b->size += size;
}

// add a new block
void add_new_block(arena_t *arena, uint64_t address, uint64_t size, long n)
{
	// add the new node to the n-th position in the list
	node *new_node = add_nth_node(arena->alloc_list, n);
	if (!new_node)
		fprintf(stderr, "This zone could not be allocated\n");

	// allocate the new block
	new_node->data_b = malloc(sizeof(block_t));
	if (!new_node->data_b)
		fprintf(stderr, "This zone could not be allocated\n");
	new_node->data_b->start_address = address;
	new_node->data_b->size = 0;

	// add the new miniblock that is generated by the new block
	new_node->data_b->miniblock_list = create_list();
	add_new_miniblock(new_node, address, size, 0);

	arena->alloc_list->list_size += size;
}

// chain two blocks
void chain_block(node *node)
{
	struct node *next = node->next;
	list_t *l1, *l2;

	l1 = (list_t *)node->data_b->miniblock_list;
	l2 = (list_t *)next->data_b->miniblock_list;

	// the second block is added to the first block
	// the size of the lists is updated
	l1->list_size += l2->list_size;
	l1->size += l2->size;
	node->data_b->size = l1->list_size;

	// find the address last miniblock in the first block
	struct node *curr = l1->head;
	while (curr->next)
		curr = curr->next;

	// chain the last miniblock from the first block 
	// to the first miniblock from the second block
	curr->next = l2->head;
	l2->head->prev = curr;

	node->next = next->next;
	next->prev = node;

	// deallocate the resources of the second block
	free(l2);
	free(next->data_b);
	free(next);
}

// return the size of a list
size_t list_size(list_t *list)
{
	return list->size;
}

int cases(node *node, const uint64_t address, const uint64_t size)
{
	uint64_t dim_node = address + size;
	uint64_t start_address = node->data_b->start_address;
	size_t size_bl = node->data_b->size;
	uint64_t dim_bl = start_address + size_bl;

	// ---------- Cases in which we cannot allocate -------------

	if (address >= start_address && address < dim_bl)
		return 0;

	if (address <= start_address && dim_node > start_address)
		return 0;

	if (node->next)
		if (dim_bl <= address && dim_node > node->next->data_b->start_address)
			return 0;

	// ---------------Cases in which we can allocate ----------------
	if (dim_node == start_address)
		return 4;

	if (!node->next) {
		if (address > dim_bl)
			return 1;

		if (dim_bl == address)
			return 3;
	} else {
		if (address > dim_bl && dim_node < node->next->data_b->start_address)
			return 1;

		if (dim_bl == address && dim_node == node->next->data_b->start_address)
			return 2;
	}

	if (dim_bl == address)
		return 3;

	if (address < dim_bl)
		return 5;

	return 0;
}

void find_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	node *curr = arena->alloc_list->head;
	node *prev = curr;

	int ok = 0;
	long pos = 0;

	// finding the position of the block in which the miniblock will be inserted
	while (curr && ok == 0) {
		ok = cases(curr, address, size);
		prev = curr;
		curr = curr->next;
		pos++;

		uint64_t dim_node = address + size;
		uint64_t start_address = prev->data_b->start_address;
		size_t size_bl = prev->data_b->size;
		uint64_t dim_bl = start_address + size_bl;

		// cases in which we cannot allocate
		if (address >= arena->arena_size) {
			printf("The allocated address is outside the size of arena\n");
			return;
		}

		if (dim_node > arena->arena_size) {
			printf("The end address is past the size of the arena\n");
			return;
		}

		if (address >= start_address && address < dim_bl && ok == 0)
			break;

		if (address <= start_address && dim_node > start_address && ok == 0)
			break;
	}

	switch (ok) {
	case 1: // allocate a new block after the current block
		add_new_block(arena, address, size, pos);
		break;
	case 2: // chain two blocks
		pos = list_size((list_t *)prev->data_b->miniblock_list);
		add_new_miniblock(prev, address, size, pos);
		chain_block(prev);
		arena->alloc_list->size--;
		arena->alloc_list->list_size += size;
		break;
	case 3: // add a new miniblock at the end of the current block
		pos = list_size((list_t *)prev->data_b->miniblock_list);
		add_new_miniblock(prev, address, size, pos);
		arena->alloc_list->list_size += size;
		break;
	case 4: // add a new miniblock at the beginning of the current block
		add_new_miniblock(prev, address, size, 0);
		prev->data_b->start_address = address;
		arena->alloc_list->list_size += size;
		break;
	case 5: // add a new block before the current block
		add_new_block(arena, address, size, pos - 1);
		break;
	default: // the zone was already allocated
		printf("This zone was already allocated.\n");
		break;
	}
}


void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	if (!arena->alloc_list) {
		arena->alloc_list = create_list();
		add_new_block(arena, address, size, 0);
		return;
	}

	if (arena->alloc_list->size == 0) {
		add_new_block(arena, address, size, 0);
		return;
	}

	find_block(arena, address, size);
}

// remove the n-th node from a list
void remove_nth_node(list_t *list, node *node, long n, int type)
{
	if (list->size == 1) {
		list->head = NULL;
	} else {
		if (n == 1) {
			list->head = node->next;
			node->next->prev = NULL;
		} else {
			if (n == (long)list->size) {
				node->prev->next = NULL;
			} else {
				node->prev->next = node->next;
				node->next->prev = node->prev;
			}
		}
	}

	list->size--;

	// deallocate the resources of the removed node
	if (type == 1) {
		list->list_size -= node->data_b->size;
		free(node->data_b->miniblock_list);
		free(node->data_b);
	} else {
		list->list_size -= node->data_mb->size;
		free(node->data_mb->rw_buffer);
		free(node->data_mb);
	}
	free(node);
}

// verify if an address is the start address of a block
void search_block(list_t *list, uint64_t address, node **node_find, long *pos)
{
	node *curr = list->head;
	*node_find = NULL;
	*pos = 0;
	while (curr) {
		(*pos)++;
		uint64_t start_address = curr->data_b->start_address;
		uint64_t dim = start_address + curr->data_b->size;
		if (address >= start_address && address < dim) {
			*node_find = curr;
			break;
		}
		curr = curr->next;
	}
}

// verify if an address is the start address of a miniblock
void
search_miniblock1(list_t *list, uint64_t address, node **node_find, long *pos)
{
	node *curr = list->head;
	*node_find = NULL;
	*pos = 0;
	while (curr) {
		(*pos)++;
		if (address == curr->data_mb->start_address) {
			*node_find = curr;
			break;
		}
		curr = curr->next;
	}
}

// deallocate a block/miniblock
void free_block(arena_t *arena, const uint64_t address)
{
	if (!arena->alloc_list) {
		printf("Invalid address for free.\n");
		return;
	}

	node *node_find_b = NULL; long pos_b = 0; // position of the block
	search_block(arena->alloc_list, address, &node_find_b, &pos_b);

	if (!node_find_b) {
		printf("Invalid address for free.\n");
		return;
	}

	list_t *l = (list_t *)node_find_b->data_b->miniblock_list;
	node *node_find_mb; long pos_mb = 0; // position of the miniblock
	search_miniblock1(l, address, &node_find_mb, &pos_mb);

	if (!node_find_mb) {
		printf("Invalid address for free.\n");
		return;
	}

	size_t size_mb = node_find_mb->data_mb->size; uint64_t new_address;
	if (!node_find_mb->next)
		new_address = 0;
	else
		new_address = node_find_mb->next->data_mb->start_address;

	remove_nth_node(l, node_find_mb, pos_mb, 2);

	if (pos_mb == 1) { // remobe the miniblock from the beginning
		node_find_b->data_b->start_address = new_address;
		node_find_b->data_b->size -= size_mb;
		arena->alloc_list->list_size -= size_mb;
		if (l->size == 0) // the block has no more miniblocks
			remove_nth_node(arena->alloc_list, node_find_b, pos_b, 1);
		return;
	}

	if (pos_mb == (long)l->size + 1) { // remove the miniblock from the end
		node_find_b->data_b->size -= size_mb;
		arena->alloc_list->list_size -= size_mb;
		return;
	}

	// ---- Remove the miniblock from the inside of the block ----
	arena->alloc_list->list_size -= size_mb;
	long x = pos_mb - 1; long y = l->size - pos_mb + 1;
	size_t total_b1 = 0, total_b2 = 0;
	node *curr = l->head;
	while (x) {
		total_b1 += curr->data_mb->size;
		curr = curr->next;
		x--;
	}

	total_b2 = node_find_b->data_b->size - total_b1 - size_mb;
	l->size = pos_mb - 1; l->list_size = total_b1;

	node_find_b->data_b->size = total_b1;

	node *new_block = malloc(sizeof(*new_block));
	new_block->data_b = malloc(sizeof(block_t));
	new_block->data_b->start_address = curr->data_mb->start_address;
	new_block->data_b->size = total_b2;

	new_block->data_b->miniblock_list = create_list();
	l = (list_t *)new_block->data_b->miniblock_list;
	l->list_size = total_b2; l->head = curr; l->size = y;

	curr->prev->next = NULL; curr->prev = NULL;
	new_block->prev = node_find_b;
	new_block->next = node_find_b->next;

	if (node_find_b->next)
		node_find_b->next->prev = new_block;
	node_find_b->next = new_block;

	arena->alloc_list->size++;
}

// verify if an address is the address of a miniblock
void
search_miniblock2(list_t *list, uint64_t address, node **node_find, long *pos)
{
	node *curr = list->head;
	*node_find = NULL;
	*pos = 0;
	while (curr) {
		(*pos)++;
		uint64_t start_address = curr->data_mb->start_address;
		uint64_t dim = start_address + curr->data_mb->size;
		if (address >= start_address && address < dim) {
			*node_find = curr;
			break;
		}
		curr = curr->next;
	}
}

void read(arena_t *arena, uint64_t address, uint64_t size)
{
	if (!arena->alloc_list) {
		printf("Invalid address for read.\n");
		return;
	}

	// ------------------ Find the address ------------------
	node *node_find_b = NULL;
	long pos_b;
	search_block(arena->alloc_list, address, &node_find_b, &pos_b);

	if (!node_find_b) {
		printf("Invalid address for read.\n");
		return;
	}

	list_t *list = (list_t *)node_find_b->data_b->miniblock_list;
	node *node_find_mb;
	long pos_mb;
	search_miniblock2(list, address, &node_find_mb, &pos_mb);

	if (!node_find_mb) {
		printf("Invalid address for read.\n");
		return;
	}

	// --------------- Verify the permissions ----------------
	node *curr = node_find_mb;
	uint64_t size_readable = 0;
	while (curr && size_readable < size) {
		size_readable += curr->data_mb->size;
		int8_t perm = curr->data_mb->perm;
		if (perm < 4) {
			printf("Invalid permissions for read.\n");
			return;
		}
		if (!(curr->data_mb->rw_buffer) && size_readable <= size) {
			printf("Invalid address for read.\n");
			return;
		}
		curr = curr->next;
	}

	if (size_readable < size) {
		size = size_readable;
		printf("Warning: size was bigger than the block size. ");
		printf("Reading %lu characters.\n", size);
	}

	size_readable = 0;

	uint64_t start_read = address - node_find_mb->data_mb->start_address;
	curr = node_find_mb;
	while (curr) {
		int8_t *buffer = (int8_t *)curr->data_mb->rw_buffer;
		long n = curr->data_mb->size;
		size_readable += n - start_read;
		if (size_readable > size) {
			n = n - (size_readable - size);
			size_readable = n - start_read;
		}
		for (long i = start_read; i < n; i++)
			printf("%c", buffer[i]);

		if (size_readable == size) {
			printf("\n");
			return;
		}

		curr = curr->next;
	}
}

void read_characters(uint64_t size)
{
	while (size) {
		getchar();
		size--;
	}
}

int8_t *text(arena_t *arena, const uint64_t address, const uint64_t size)
{
	getchar();
	if (!arena->alloc_list) {
		printf("Invalid address for write.\n");
		read_characters(size);
		return NULL;
	}

	// ------------------ Find the address ------------------
	node *node_find_b = NULL;
	long pos_b;
	search_block(arena->alloc_list, address, &node_find_b, &pos_b);

	if (!node_find_b) {
		printf("Invalid address for write.\n");
		read_characters(size);
		return NULL;
	}

	list_t *list = (list_t *)node_find_b->data_b->miniblock_list;
	node *node_find_mb;
	long pos_mb;
	search_miniblock2(list, address, &node_find_mb, &pos_mb);

	if (!node_find_mb) {
		printf("Invalid address for write.\n");
		read_characters(size);
		return NULL;
	}

	// --------------- 	Veify the permissions ----------------
	node *curr = node_find_mb;
	uint64_t size_readable = 0;
	while (curr && size_readable < size) {
		size_readable += curr->data_mb->size;
		int8_t perm = curr->data_mb->perm;
		if (perm < 2 || perm == 4 || perm == 5) {
			printf("Invalid permissions for write.\n");
			read_characters(size);
			return NULL;
		}
		curr = curr->next;
	}

	size_readable = size;
	uint64_t rest = 0;
	uint64_t start_address = address - list->head->data_mb->start_address;

	uint64_t block_size = node_find_b->data_b->size - start_address;
	if (block_size < size) {
		printf("Warning: size was bigger than the block size. ");
		printf("Writing %lu characters.\n", block_size);
		size_readable = block_size;
		rest = size - size_readable;
	}

	int8_t *buffer = malloc((size_readable + 1) * sizeof(int8_t));
	long i = 0; char c;
	while (size_readable) {
		c = getchar();
		buffer[i] = c;
		i++;
		size_readable--;
	}
	buffer[i] = '\0';
	if (rest != 0)
		read_characters(rest - 1);

	return buffer;
}

void
write(arena_t *arena, const uint64_t address, const uint64_t size, int8_t *data)
{
	if (!data)
		return;

	// ------------------ Find the address ------------------
	node *node_find_b = NULL;
	long pos_b;
	search_block(arena->alloc_list, address, &node_find_b, &pos_b);

	list_t *list = (list_t *)node_find_b->data_b->miniblock_list;
	node *node_find_mb;
	long pos_mb;
	search_miniblock2(list, address, &node_find_mb, &pos_mb);

	// writing the data
	uint64_t size_data = size;
	long n = size_data;
	node *curr = node_find_mb;
	uint64_t i = 0;
	uint64_t start_address = address - node_find_mb->data_mb->start_address;
	while (n > 0) {
		uint64_t size_mb = curr->data_mb->size;
		n -= size_mb - start_address;
		int8_t *buffer = (int8_t *)curr->data_mb->rw_buffer;

		for (uint64_t k = start_address; k < size_mb; k++) {
			if (i + k > size_data)
				break;
			buffer[k] = data[i + k];
		}

		i += size_mb - start_address;
		start_address = 0;
		curr = curr->next;
	}
}

void printf_perm(int8_t perm)
{
	if (perm == 0)
		printf("---\n");
	if (perm == 1)
		printf("--X\n");
	if (perm == 2)
		printf("-W-\n");
	if (perm == 3)
		printf("-WX\n");
	if (perm == 4)
		printf("R--\n");
	if (perm == 5)
		printf("R-X\n");
	if (perm == 6)
		printf("RW-\n");
	if (perm == 7)
		printf("RWX\n");
}

void pmap(const arena_t *arena)
{
	// ------------------- Arena size -------------------------
	unsigned long long arena_size = (unsigned long long)arena->arena_size;
	printf("Total memory: 0x%llX bytes\n", arena_size);

	// if there are no allocated blocks
	if (!arena->alloc_list) {
		printf("Free memory: 0x%llX bytes\n", arena_size);
		printf("Number of allocated blocks: 0\n");
		printf("Number of allocated miniblocks: 0\n");
		return;
	}

	// --------------------- Free memory ---------------------------
	unsigned long long free_mem;
	free_mem = (unsigned long long)arena_size - arena->alloc_list->list_size;
	printf("Free memory: 0x%llX bytes\n", free_mem);

	// --------------- Number of allocated blocks ---------------------
	unsigned long long size_list = (unsigned long long)arena->alloc_list->size;
	printf("Number of allocated blocks: %llu\n", size_list);

	// calculate the number of miniblocks
	unsigned long long nr_minib = 0;
	node *curr = arena->alloc_list->head;
	while (curr) {
		list_t *l = (list_t *)curr->data_b->miniblock_list;
		nr_minib += (unsigned long long)l->size;
		curr = curr->next;
	}

	// --------------- The number of allocated miniblocks ----------------
	printf("Number of allocated miniblocks: %llu\n", nr_minib);

	node *curr1, *curr2;
	curr1 = arena->alloc_list->head;
	unsigned long long i = 0;

	while (curr1) {
		i++;
		// display the current block
		printf("\nBlock %llu begin\n", i);

		list_t *l = (list_t *)curr1->data_b->miniblock_list;
		unsigned long long start_address, size;
		start_address = (unsigned long long)curr1->data_b->start_address;
		size = (unsigned long long)curr1->data_b->start_address + l->list_size;
		printf("Zone: 0x%llX - 0x%llX\n", start_address, size);

		curr2 = l->head;
		unsigned long long j = 1;

		while (curr2) {
			unsigned long long start_address, size;
			start_address = (unsigned long long)curr2->data_mb->start_address;
			size = (unsigned long long)start_address + curr2->data_mb->size;
			printf("Miniblock %llu:", j);
			printf("\t\t0x%llX\t\t-\t\t0x%llX\t\t| ", start_address, size);

			// show the permissions of the miniblock
			printf_perm(curr2->data_mb->perm);

			curr2 = curr2->next;
			j++;
		}
		printf("Block %llu end\n", i);
		curr1 = curr1->next;
	}
}

int permissions_cases(char *s)
{
	if (strcmp(s, "PROT_NONE") == 0)
		return 0;

	if (strcmp(s, "PROT_READ") == 0)
		return 4;

	if (strcmp(s, "PROT_WRITE") == 0)
		return 2;

	if (strcmp(s, "PROT_EXEC") == 0)
		return 1;
	return 0;
}

// change the permissions of a miniblock
void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
{
	if (!arena->alloc_list) {
		printf("Invalid address for mprotect.\n");
		return;
	}

	// ------------------ Se cauta adresa ------------------
	node *node_find_b = NULL;
	long pos_b;
	search_block(arena->alloc_list, address, &node_find_b, &pos_b);

	if (!node_find_b) {
		printf("Invalid address for mprotect.\n");
		return;
	}

	list_t *list = (list_t *)node_find_b->data_b->miniblock_list;
	node *node_find_mb;
	long pos_mb;
	search_miniblock1(list, address, &node_find_mb, &pos_mb);

	if (!node_find_mb) {
		printf("Invalid address for mprotect.\n");
		return;
	}

	char *p = strtok((char *)permission, " |");
	node_find_mb->data_mb->perm = 0;
	while (p) {
		int perm = permissions_cases(p);
		if (perm == 0)
			node_find_mb->data_mb->perm = 0;
		else
			node_find_mb->data_mb->perm += perm;
		p = strtok(NULL, " |");
	}
}

int convert(char s[])
{
	if (strcmp(s, "ALLOC_ARENA") == 0)
		return 1;

	if (strcmp(s, "DEALLOC_ARENA") == 0)
		return 2;

	if (strcmp(s, "ALLOC_BLOCK") == 0)
		return 3;

	if (strcmp(s, "FREE_BLOCK") == 0)
		return 4;

	if (strcmp(s, "READ") == 0)
		return 5;

	if (strcmp(s, "WRITE") == 0)
		return 6;

	if (strcmp(s, "PMAP") == 0)
		return 7;

	if (strcmp(s, "MPROTECT") == 0)
		return 8;

	return -1;
}
