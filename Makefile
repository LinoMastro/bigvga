bin = bigvga.exe
src = $(wildcard *.c)
obj = $(src:.c=.o)

CC = ia16-elf-gcc
LDLIBS = -li86

$(bin): $(obj)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

.PHONY: clean
clean:
	rm -f $(bin) $(obj)
