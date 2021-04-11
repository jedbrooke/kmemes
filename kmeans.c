#include "kmeans.h"

int sum(int* a, int n){
    int sum = 0;
    for (int i = 0; i < n; i++){
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

void divide_vector_by_scalar(feature_type* dest, uint* v, int s, int n) {
    for (int i = 0; i < n; i++){
        // TODO we can parellelize this too
        dest[i] = (feature_type) (v[i] / s);
    }
}

void vector_subtract(int* dest, feature_type* a, feature_type* b, int n) {
    for (int i = 0; i < n; i++) {
        dest[i] = (int) (a[i] - b[i]);
    }
}

void vector_elementwise_square(int* dest, int* a, int n) {
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
feature_type** kmeans(feature_type** data, int N, int f_size, int k) {
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
    float Jscore = 1;
    float threshold = 1e-6;

    bool keep_looping = true;

    int max_iterations = 1000;
    int iterations = 0;

    feature* features = (feature*) malloc(N * sizeof(feature));
    for (int i = 0; i < N; i++) {
        features[i].features = data[i];
        // random initialize group
        features[i].group = rand() % k;
    }

    uint* D = (uint*) malloc(N * sizeof(uint));

    int* temp = (int*) malloc(f_size * sizeof(int));
    int temp_d;

    // main kmeans iteration loop
    while(keep_looping) {
        // reset Zsums and Zcounts to 0;
        for (int i = 0; i < k; i++) {
            bzero(Zsums[i],f_size * sizeof(feature_type));
        }
        bzero(Zcounts,f_size * sizeof(uint));

        for (int i = 0; i < N; i++) {
            // when we multi-thread it later we will need to implement proper locking
            add_vector_to_dest(Zsums[features[i].group],features[i].features,f_size);
            Zcounts[features[i].group]++;
        }

        for (int i = 0; i < k; i++) {
            divide_vector_by_scalar(Z[i],Zsums[i],Zcounts[i],f_size);
        }

        //reset distances to max_val
        memset(D, 0xFF, N * sizeof(uint));

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < k; j++) {
                vector_subtract(temp,features[i].features, Z[j], f_size);
                vector_elementwise_square(temp,temp,f_size);
                temp_d = sum(temp, f_size);
                if(temp_d < D[i]) {
                    D[i] = temp_d;
                    features[i].group = j;
                }

            }
        }

        Jscore = (float) sum(D,N) / N;

        keep_looping = fabs(Jscore - prev_j) < (1e-6 * Jscore) || iterations++ > max_iterations;
    }

    return Z;
}