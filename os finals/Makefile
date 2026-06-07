CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
TARGETS = scheduler

all: $(TARGETS)

scheduler: scheduler.c
	$(CC) $(CFLAGS) -o scheduler scheduler.c

run1: scheduler
	./scheduler Input1.txt

run2: scheduler
	./scheduler Input2.txt

clean:
	rm -f $(TARGETS) *.o

.PHONY: all run1 run2 clean
