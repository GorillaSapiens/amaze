CC = g++

# enable all warnings
CFLAGS = -Wall
CFLAGS += -Wall -Wextra -Wpedantic \
          -Wshadow -Wpointer-arith -Wcast-qual \
          -Wformat=2 -Wundef
# enable debugging
CFLAGS += -g

# generate dependencies
DEPFLAGS += -MMD -MP

OBJS = \
   main.o \
   ansi.o \
   binding.o \
   chooselist.o \
   messages.o \
   mprintf.o \
   object.o \
   sparse_array.o \
   sparse_chars.o

DEPS := $(OBJS:.o=.d)

all: amaze

amaze: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lm

%.o: %.cc
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

%.d: %.cc
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
