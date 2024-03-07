// COPYRIGHT: Larisa Florea

#pragma once
#include <stdio.h>
#include <inttypes.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

typedef struct block_t block_t;
typedef struct miniblock_t miniblock_t;
typedef struct node node;

struct node {
	node *next, *prev;
	union {
		block_t *data_b;
		miniblock_t *data_mb;
	};
};

typedef struct {
	node *head;
	size_t size;
	uint64_t list_size;
} list_t;

struct block_t {
	uint64_t start_address;
	size_t size;
	void *miniblock_list;
};

struct miniblock_t {
	uint64_t start_address;
	size_t size;
	uint8_t perm;
	void *rw_buffer;
};

typedef struct {
	uint64_t arena_size;
	list_t *alloc_list;
} arena_t;

arena_t *alloc_arena(const uint64_t size);

void dealloc_arena(arena_t *arena);

list_t *create_list(void);

node *get_nth_node(list_t *list, long n);

node *add_nth_node(list_t *list, long n);

void add_new_miniblock(node *node, uint64_t address, uint64_t size, long n);

void add_new_block(arena_t *arena, uint64_t address, uint64_t size, long n);

void chain_block(node *node);

size_t list_size(list_t *list);

int cases(node *node, const uint64_t address, const uint64_t size);

void find_block(arena_t *arena, const uint64_t address, const uint64_t size);

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size);

void remove_nth_node(list_t *list, node *node, long n, int type);

void search_block(list_t *list, uint64_t address, node **node_find, long *pos);

void
search_miniblock1(list_t *list, uint64_t address, node **node_find, long *pos);

void free_block(arena_t *arena, const uint64_t address);

void
search_miniblock2(list_t *list, uint64_t address, node **node_find, long *pos);

void read(arena_t *arena, uint64_t address, uint64_t size);

void read_characters(uint64_t size);

int8_t *text(arena_t *arena, const uint64_t address, const uint64_t size);

void write(arena_t *arena, const uint64_t address,
		   const uint64_t size, int8_t *data);

void printf_perm(int8_t perm);

void pmap(const arena_t *arena);

int permissions_cases(char *s);

void mprotect(arena_t *arena, uint64_t address, int8_t *permission);

int convert(char s[]);

