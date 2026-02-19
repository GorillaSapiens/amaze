CC = gcc
CFLAGS = -g -Wall

OBJS = \
   main.o \
   entity.o \
   sparse_array.o

all: amaze

amaze: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) amaze
