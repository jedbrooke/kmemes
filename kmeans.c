#include "kmeans.h"
#include <stdio.h>

uint sum(uint* a, uint n){
    uint sum = 0;
    for (uint i = 0; i < n; i++){
        sum += a[i];
    }
    return sum;
}

void add_vector_to_dest(uint* dest, feature_type* a, int n) {
    // TODO: add proper mutex locking for multi-threading
    // experiment with coarse locking (lock each Zsums[i])
    // or fine locking (lock each Zums[i][j])
    for (int i = 0; i < n; i++) {
        dest[i] += a[i];
    }
}

void divide_vector_by_scalar(feature_type* dest, uint* v, uint s, uint n) {
    for (int i = 0; i < n; i++){
        // TODO we can parellelize this too
        dest[i] = (feature_type) (v[i] / s);
    }
}

void vector_subtract_abs(uint* dest, feature_type* a, feature_type* b, uint n) {
    for (int i = 0; i < n; i++) {
        dest[i] = (a[i] - b[i]);
    }
}

void vector_elementwise_square(uint* dest, uint* a, uint n) {
    for(int i = 0; i < n; i++) {
        dest[i] = a[i] * a[i];
    }
}

/*  
    takes in an array of N feature vectors,
    each with f features.
    returns an array of k feature vectors
    from the result of k means.

*/
observation* kmeans(observation* data, int N, int f_size) {
    printf("running kmeans!!\n");
    // Z holds the group representatives
    feature_type** Z = (feature_type**) malloc(k * sizeof(feature_type*));
    // store the intermediate sums for Z while summing up to find the means
    uint** Zsums = (uint**) malloc(k * sizeof(uint*));
    for (int i = 0; i < k; i ++) {
        Z[i] = (feature_type*) malloc(f_size * sizeof(feature_type));
        Zsums[i] = (uint*) malloc(f_size * sizeof(uint));
    }
    // store the count of the number of members of each mean to find the average
    uint* Zcounts = (uint*) malloc(k * sizeof(uint));

    float prev_j = 0;
    float Jscore = -1;
    float threshold = 10e-3;

    bool stop_looping = false;

    int max_iterations = 100;
    int iterations = 0;

    for (int i = 0; i < N; i++) {
        // random initialize group
        // not random for now so we can get consistent results while testing
        data[i].group = i % k;
    }

    uint* D = (uint*) malloc(N * sizeof(uint));

    uint* temp = (uint*) malloc(f_size * sizeof(uint));
    uint temp_d;

    // main kmeans iteration loop
    while(!stop_looping) {
        printf("epoch %d\n",iterations);
        // reset Zsums and Zcounts to 0;

        for (int i = 0; i < k; i++) {
            bzero(Zsums[i],f_size * sizeof(uint));
        }
        bzero(Zcounts,k * sizeof(uint));

        for (int i = 0; i < N; i++) {
            // when we multi-thread it later we will need to implement proper locking
            add_vector_to_dest(Zsums[data[i].group],data[i].features,f_size);
            Zcounts[data[i].group]++;
        }

        for (int i = 0; i < k; i++) {
            divide_vector_by_scalar(Z[i],Zsums[i],Zcounts[i],f_size);
        }

        //reset distances to max_val
        memset(D, -1, N * sizeof(uint));

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < k; j++) {
                vector_subtract_abs(temp,data[i].features, Z[j], f_size);
                vector_elementwise_square(temp,temp,f_size);
                temp_d = sum(temp, f_size);
                if(temp_d < D[i]) {
                    D[i] = temp_d;
                    data[i].group = j;
                }

            }
        }

        Jscore = abs((float) sum(D,N) / N);
        printf("J = %0.5f\n",Jscore);
        printf("Diff = %0.5f\n",fabs(Jscore - prev_j));
        printf("Thresh = %0.5f\n",(threshold * Jscore));
        iterations++;
        stop_looping = fabs(Jscore - prev_j) < fabs(threshold * Jscore) || iterations >= max_iterations;
        prev_j = Jscore;
    }
    observation* means = (observation*) malloc(k * sizeof(observation));
    for(size_t i = 0; i < k; i++) {
        means[i].features = Z[i];
        means[i].group = i;
    }
    return means;
}