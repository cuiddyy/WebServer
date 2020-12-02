CFLAGS+=-pthread
LDFLAGS+=-pthread


all:relayer

relayer:relayer.o main.o
	gcc $^ -o $@

clean:
	rm -rf *.o relayer
