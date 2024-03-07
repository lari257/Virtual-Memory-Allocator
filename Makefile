# Florea Larisa-Elena Grupa 313CA

# compiler setup
CC=gcc
CFLAGS=-Wall -Wextra -std=c99

# define targets
TARGETS = vma

build: $(TARGETS)

run_vma:
	./run_vma

vma: vma.o main.c
	$(CC) $(CFLAGS) vma.o main.c -o vma

vma.o: vma.c
	$(CC) -c $(CFLAGS) vma.c

pack:
	zip -FSr 313CA_FloreaLarisa_Elena_Tema1.zip README Makefile *.c *.h

clean:
	rm -f *.o $(TARGETS)

.PHONY: pack clean