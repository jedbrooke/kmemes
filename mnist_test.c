#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "kmeans.h"

const char* training_data = "mnist.dat";
const char* output_file = "means.csv";
const int MNIST_DATA_SIZE = 28 * 28;

int main(int argc, char const *argv[])
{
    FILE* data_fp = fopen(training_data,"rb");
    if(!data_fp) {
        perror("fopen");
        exit(1);
    }
    int data_fd = fileno(data_fp);

    int N = 60000;
    feature_type* data_all = (feature_type *) malloc(N * MNIST_DATA_SIZE * sizeof(feature_type*));
    observation* data = (observation*) malloc(N * sizeof(feature_type*));
    for (int i = 0; i < N; i++) {
        // read in one digit
        read(data_fd,&data_all[i * MNIST_DATA_SIZE],MNIST_DATA_SIZE * sizeof(feature_type));
        data[i].features = &data_all[i * MNIST_DATA_SIZE];
    }

    fclose(data_fp);

    // run kmeans
    observation* means = kmeans(data, N, MNIST_DATA_SIZE);

    // save the kmeans to a file so we can view them with a python script
    FILE* output_fp = fopen(output_file,"w");
    if(!output_fp) {
        perror("fopen");
        exit(1);
    }
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < MNIST_DATA_SIZE; j++) {
            fprintf(output_fp,"%u,",means[i].features[j]);
        }
        fprintf(output_fp,"\n");
    }
    fflush(output_fp);
    fclose(output_fp);
    
    return 0;
}
