xcc: xcc.c
	docker run -it --rm -v $(PWD):/xcc xcc:local gcc -o xcc xcc.c

.PHONY: docker-build
docker-build: Dockerfile
	docker build -t xcc:local .

.PHONY: test
test: xcc docker-build
	docker run -it --rm -v $(PWD):/xcc xcc:local ./xcc -test
	docker run -it --rm -v $(PWD):/xcc xcc:local ./test.sh

.PHONY: run
run: docker-build
	docker run -it --rm -v $(PWD):/xcc xcc:local

clean:
	@rm -f xcc *.o *~ tmp*
