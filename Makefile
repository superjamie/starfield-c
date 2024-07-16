CFLAGS  = -fanalyzer -fno-omit-frame-pointer -g3 -std=c11 -O3
CFLAGS += -Wall -Wextra -Wpedantic -Wmissing-prototypes -Werror=vla
CFLAGS += $(shell pkg-config --cflags sdl2)
LDLIBS  = -lm
LDLIBS += $(shell pkg-config --libs sdl2)

EXEC    = stars
SRCS    = stars.c
OBJS    = stars.o

$(EXEC): $(OBJS)
	$(CC) -o $@ $^ $(LDLIBS)

%.o: %.c
	$(CC) -c -o $@ $^ $(CFLAGS)

.PHONY: clean check

clean:
	rm -f $(OBJS) $(EXEC)

check: $(SRCS)
	cppcheck --enable=all --std=c11 --suppress=missingInclude $^
