CFLAGS+=-pthread
LDFLAGS+=-pthread
all:myse

myse:main.o myse.o
	gcc $^ -o $@ $(CFLAGS) $(LDFLAGS)

clean:
	rm -rf *.o myse

