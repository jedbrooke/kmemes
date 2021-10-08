CC=gcc
CFLAGS=-Wall -O3
CPPFLAGS=--std=c++17 -g
CXX=g++
NVCC=nvcc
NVFLAGS=-O3 -arch=compute_52

mnist:
	# lifted from https://github.com/sorki/python-mnist/blob/master/bin/mnist_get_data.sh
	wget --recursive --level=1 --cut-dirs=3 --no-host-directories \
	--directory-prefix=mnist --accept '*.gz' http://yann.lecun.com/exdb/mnist/

mnist.dat: mnist
	python3 mnist_download.py


kmeans.o: kmeans.c kmeans.h
	$(CC) -c -o kmeans.o kmeans.c $(CFLAGS)

kmeans_gpu.o: kmeans.h kmeans.cu
	$(NVCC) -c -o kmeans_gpu.o kmeans.cu $(NVFLAGS)

mnist_test: kmeans.o mnist_test.c mnist.dat
	$(CC) -o mnist_test mnist_test.c kmeans.o 

mnist_gpu: kmeans_gpu.o mnist.dat mnist_test_gpu.cpp
	$(CXX) -o mnist_gpu mnist_test_gpu.cpp kmeans_gpu.o -lcudart $(CFLAGS) $(CPPFLAGS)

memes_small_test: kmeans_gpu.o memes_small_test.cpp kmeans.o
	$(CXX) -o memes_small_test memes_small_test.cpp kmeans_gpu.o kmeans.o -lcudart -lpng $(CFLAGS) $(CPPFLAGS)

1000memes:
	mkdir 1000memes
	# for f in $$(ls memes | shuf | head -n 10000); do convert memes/$$f -colorspace RGB -alpha remove -type TrueColor -resize 512x512! -depth 8 1000memes/$$(basename $$f | sed 's/\(.*\)\..*/\1/').png; done
	ls memes | shuf | head -n 10000 | parallel convert memes/{} -colorspace RGB -alpha remove -type TrueColor -resize 512x512! -depth 8 1000memes/$$(basename {} | sed 's/\(.*\)\..*/\1/').png

.PHONY: clean
clean:
	rm -f *.o mnist_test mnist_gpu memes_small_test
