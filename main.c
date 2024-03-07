// COPYRIGHT: Larisa Florea

#include "vma.h"

int main(void)
{
	char command[50];
	int exit = 1;
	unsigned long long size, a, b;
	arena_t *arena = NULL;
	int8_t *data = NULL, permission[200];

	while (exit) {
		scanf("%s", command);

		switch (convert(command)) {
		case 1: // ALLOC_ARENA
			scanf("%llu", &size);
			arena = alloc_arena(size);
			break;

		case 2: // DEALLOC_ARENA
			dealloc_arena(arena);
			free(arena);
			exit = 0;
			break;

		case 3: // ALLOC_BLOCK
			scanf("%llu%llu", &a, &b);
			alloc_block(arena, a, b);
			break;

		case 4: // FREE_BLOCK
			scanf("%llu", &a);
			free_block(arena, a);
			break;

		case 5: // READ
			scanf("%llu%llu", &a, &b);
			read(arena, a, b);
			break;

		case 6: // WRITE
			scanf("%llu%llu", &a, &b);
			data = text(arena, a, b);
			if (data)
				b = strlen((char *)data);
			write(arena, a, b, data);
			if (data)
				free(data);
			break;

		case 7: // PMAP
			pmap(arena);
			break;

		case 8: // MPROTECT
			scanf("%llu", &a);
			scanf("%[^\n]", permission);
			mprotect(arena, a, permission);
			break;

		default: // INVALID COMMAND
			printf("Invalid command. Please try again.\n");
			break;
		}
		getchar();
	}

	return 0;
}

