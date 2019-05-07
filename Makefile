CC=gcc
SRCS= $(wildcard *.c)
OBJS = $(SRCS:.c=.exe)
LIBS = -lpthread


all:$(OBJS)

%.exe:%.c
	$(CC) $^ $(LIBS) -o $@

clean:
	rm -f ./*.o ./*.exe
