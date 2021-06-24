CC=gcc
CFLAGS=-Wall -O3 -g

mnist:
	# lifted from https://github.com/sorki/python-mnist/blob/master/bin/mnist_get_data.sh
	wget --recursive --level=1 --cut-dirs=3 --no-host-directories \
	--directory-prefix=mnist --accept '*.gz' http://yann.lecun.com/exdb/mnist/

mnist.dat: mnist
	python3 mnist_download.py


kmeans.o: kmeans.c kmeans.h
	$(CC) -c -o kmeans.o kmeans.c $(CFLAGS)

kmeans_omp.o: kmeans.c kmeans.h
	$(CC) -c -o kmeans_omp.o kmeans.c -DOMP -fopenmp

kmeans_pthread.o: kmeans.c kmeans.h
	$(CC) -c -o kmeans_pthread.o kmeans.c $(CFLAGS) -DPTHREAD -lpthread



mnist_test: kmeans.o mnist_test.c mnist.dat
	$(CC) -o mnist_test mnist_test.c kmeans.o

mnist_test_omp: kmeans_omp.o mnist_test.c mnist.dat
	$(CC) -o mnist_test mnist_test.c kmeans_omp.o -DOMP -fopenmp

mnist_test_pthread: kmeans_pthread.o mnist_test.c mnist.dat
	$(CC) -o mnist_test mnist_test.c kmeans_pthread.o -DPTHREAD -lpthread



.PHONY: clean
clean:
	rm -f *.o mnist_test
