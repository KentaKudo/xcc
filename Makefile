CC=gcc
CFLAGS=-Wall -std=c11
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

xcc: $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS)

$(OBJS): xcc.h

.PHONY: test
test: xcc test.sh test/foo.c
	@gcc -c -o foo.o test/foo.c
	./xcc -test
	./test.sh

.PHONY: docker-build
docker-build: Dockerfile
	docker build -t xcc:local .

.PHONY: run
run: docker-build
	docker run -it --rm -v $(PWD):/xcc xcc:local

host-build: docker-build
	docker run --rm -v $(PWD):/xcc xcc:local make xcc

host-test: docker-build host-build
	docker run --rm -v $(PWD):/xcc xcc:local make test

clean:
	@rm -f xcc *.o *~ tmp*
