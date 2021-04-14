CC=gcc
CFLAGS=-Wall -O3

mnist:
	# lifted from https://github.com/sorki/python-mnist/blob/master/bin/mnist_get_data.sh
	wget --recursive --level=1 --cut-dirs=3 --no-host-directories \
	--directory-prefix=mnist --accept '*.gz' http://yann.lecun.com/exdb/mnist/

mnist.dat: mnist
	python3 mnist_download.py


kmeans.o: kmeans.c kmeans.h
	$(CC) -c -o kmeans.o kmeans.c $(CFLAGS)

mnist_test: kmeans.o mnist_test.c mnist.dat
	$(CC) -o mnist_test mnist_test.c kmeans.o


.PHONY: clean
clean:
	rm -f *.o mnist_test
