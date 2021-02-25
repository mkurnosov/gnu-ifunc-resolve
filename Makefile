prog := gnu_ifunc_resolve
prog_objs := gnu_ifunc_resolve.o

CC := gcc
CFLAGS := -Wall -O0 -std=c99
LDFLAGS := -lelf -ldl -lm

.PHONY: all clean $(prog)

all: $(prog)

$(prog): $(prog_objs)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm *.o $(prog)
