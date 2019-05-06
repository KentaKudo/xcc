xcc: xcc.c

test: xcc
    echo "TODO: test target"

.PHONY: run
run:
    docker run -it --rm --name xcc -v $(PWD):/xcc xcc

clean:
    rm -f xcc *.o *~ tmp*
