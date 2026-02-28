CC = gcc

# enable all warnings
CFLAGS = -Wall
# enable debugging
CFLAGS += -g

# generate dependencies
DEPFLAGS += -MMD -MP

OBJS = \
   main.o \
   ansi.o \
   chooselist.o \
   object.o \
   sparse_array.o \
   sparse_chars.o

DEPS := $(OBJS:.o=.d)

all: amaze

amaze: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

%.o: %.c
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

%.d: %.c
	$(CC) $(CFLAGS) -MM -MP -MT $(@:.d=.o) -MF $@ $<

.PHONY: depend
depend: $(DEPS)
	@echo -n # prevent "make" from erroneously saying "nothing to be done

.PHONY: clean
clean:
	rm -f $(OBJS) amaze

.PHONY: squeaky
squeaky:
	rm -f $(OBJS) $(DEPS) amaze

-include $(DEPS)
