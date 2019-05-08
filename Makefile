xcc: xcc.c
	echo "TODO: build"

test: xcc
	echo "TODO: test target"

.PHONY: docker-build
docker-build: Dockerfile
	docker build -t xcc:local .

.PHONY: run
run: docker-build
	docker run -it --rm --name xcc -v $(PWD):/xcc xcc:local

clean:
	rm -f xcc *.o *~ tmp*
