#include <iostream>
#include <fstream>
#include <string>

#include "kmeans.h"

const std::string training_data_path("mnist.dat");
const std::string output_file_path("means_gpu.csv");
const size_t MNIST_DATA_H = 28;
const size_t MNIST_DATA_W = 28;
const size_t MNIST_DATA_D = 3;
const size_t N = 60000;

int main(int argc, char const *argv[])
{
    observation observations[N];
    std::ifstream data_file(training_data_path, std::ifstream::binary);
    if (data_file.is_open()) {
        for (size_t i = 0; i < N; i ++ ) {
            observations[i].features = (feature_type*) malloc(MNIST_DATA_H * MNIST_DATA_W * MNIST_DATA_D * sizeof(feature_type));
            data_file.read((char*)observations[i].features, MNIST_DATA_H * MNIST_DATA_W * sizeof(feature_type));
            // copy the BW data to fill RGB data
            for(size_t d = 1; d < MNIST_DATA_D; d++) {
                memcpy(&observations[i].features[d*MNIST_DATA_H*MNIST_DATA_W],observations[i].features,MNIST_DATA_H * MNIST_DATA_W * sizeof(feature_type));
            }

        }
    }
    data_file.close();

    observation* means = kmeans_gpu(observations, N, MNIST_DATA_H, MNIST_DATA_W, MNIST_DATA_D);
    std::ofstream output_file(output_file_path, std::ofstream::out);
    if (output_file.is_open()) {
        for(size_t i = 0; i < k; i++) {
            for(size_t j = 0; j < MNIST_DATA_H * MNIST_DATA_W; j++) {
                output_file << (uint) means[i].features[j] << ",";
            }
            output_file << std::endl;
        }
    }
    output_file.flush();
    output_file.close();

    return 0;
}
