CC=g++
CFLAGS=-I.
CPPFLAGS=-std=gnu++17
DEPS=
OBJ=server.o
USERID=305101337

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) $(CPPFLAGS)

all: server
server: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(CPPFLAGS)

clean:
	rm -rf *.o server *.tar.gz

dist: tarball
tarball: clean
	tar -cvzf /tmp/$(USERID).tar.gz --exclude=./.vagrant . && mv /tmp/$(USERID).tar.gz .