CC      = gcc
CFLAGS  = -fgnu89-inline -fno-omit-frame-pointer -g3 -std=c11 -O0
CFLAGS += -Wall -Wextra -Wpedantic -Wmissing-prototypes -Werror=vla
LDLIBS  = -lm -lalleg

EXEC    = stars.exe
SOURCES = $(wildcard *.c)
OBJECTS = $(SOURCES:.c=.o)
HEADERS = display.h

$(EXEC): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ $(LOADLIBES) $(LDLIBS) -o $(EXEC)

#%.o: %.c $(HEADERS)
#	$(COMPILE.c) $(OUTPUT_OPTION) $<

.PHONY: clean check

clean:
	rm -f $(OBJECTS) $(EXEC)

check: $(SOURCES) $(HEADERS)
	cppcheck --enable=all --std=c11 --suppress=missingInclude $^

