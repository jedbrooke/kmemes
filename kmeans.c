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


#ifdef PTHREAD
void* big_sum(void* args) {
    big_sum_args* values = (big_sum_args*) args;
    // printf("in big_sum for thread %d\n",values->id);
    for (int i = values->id; i < values->N; i += values->stride) {
        pthread_mutex_lock(&values->Zsums_locks[values->features[i].group]);   
        add_vector_to_dest(values->Zsums[values->features[i].group],values->features[i].features,values->f_size);
        values->Zcounts[values->features[i].group]++;
        pthread_mutex_unlock(&values->Zsums_locks[values->features[i].group]);
    }
    return NULL;
}
#endif


/*  
    takes in an array of N feature vectors,
    each with f features.
    returns an array of k feature vectors
    from the result of k means.

*/
#if defined(OMP) || defined(PTHREAD)
    feature_type** kmeans(feature_type** data, int N, int f_size, int k, int num_threads) {
#else
    feature_type** kmeans(feature_type** data, int N, int f_size, int k) {
    int num_threads = 1;
#endif
    printf("running kmeans with %d threads!!\n",num_threads);
    // Z holds the group representatives
    feature_type** Z = (feature_type**) malloc(k * sizeof(feature_type*));
    // store the intermediate sums for Z while summing up to find the means
    uint** Zsums = (uint**) malloc(k * sizeof(uint*));
 
    feature* features = (feature*) malloc(N * sizeof(feature));
    for (int i = 0; i < N; i++) {
        features[i].features = data[i];
        // random initialize group
        // not random for now so we can get consistent results while testing
        features[i].group = i % k;
    }


    for (int i = 0; i < k; i ++) {
        Z[i] = (feature_type*) malloc(f_size * sizeof(feature_type));
        Zsums[i] = (uint*) malloc(f_size * sizeof(uint));
    }
    // store the count of the number of members of each mean to find the average
    uint* Zcounts = (uint*) malloc(k * sizeof(uint));

    float prev_j = 0;
    float Jscore = -1;
    float threshold = 1e-6;

    bool stop_looping = false;

    int max_iterations = 100;
    int iterations = 0;

    

    uint* D = (uint*) malloc(N * sizeof(uint));

    uint* temp = (uint*) malloc(f_size * sizeof(uint));
    uint temp_d;

#ifdef OMP
    // omp_lock_t* Zsum_locks = (omp_lock_t*) malloc(k * sizeof(omp_lock_t));

    // for (int i = 0; i < k; i++) {
        // omp_init_lock(&Zsum_locks[i]);
    // }

    int tid;
#elif PTHREAD
    pthread_mutex_t* Zsums_locks = (pthread_mutex_t*) malloc(k * sizeof(pthread_mutex_t));
    for (int i = 0; i < k; i++) {
        pthread_mutex_init(&Zsums_locks[i],NULL);
    }
    big_sum_args* big_sum_args_list = (big_sum_args*) malloc(num_threads * sizeof(big_sum_args));
    for (int i = 0; i < num_threads; i++){
        big_sum_args_list[i].f_size = f_size;
        big_sum_args_list[i].features = features;
        big_sum_args_list[i].id = i;
        big_sum_args_list[i].N = N;
        big_sum_args_list[i].stride = num_threads;
        big_sum_args_list[i].Zcounts = Zcounts;
        big_sum_args_list[i].Zsums = Zsums;
        big_sum_args_list[i].Zsums_locks = Zsums_locks;
    }

#endif

    // main kmeans iteration loop
    while(!stop_looping) {
        printf("epoch %d\n",iterations);
        // reset Zsums and Zcounts to 0;

        for (int i = 0; i < k; i++) {
            bzero(Zsums[i],f_size * sizeof(uint));
        }
        bzero(Zcounts,k * sizeof(uint));


#ifdef OMP
    #pragma omp parallel private(tid)
        {
            tid = omp_get_thread_num();
            if (tid < num_threads) {
                printf("starting thread %d\n",tid);
                for (int i = tid; i < N; i+=num_threads) {
                    // omp_set_lock(&Zsum_locks[features[i].group]);

                    add_vector_to_dest(Zsums[features[i].group],features[i].features,f_size);
                    Zcounts[features[i].group]++;

                    // omp_unset_lock(&Zsum_locks[features[i].group]);
                }
            }
        }
    /* end parallel */
#elif PTHREAD
    pthread_t* threads = (pthread_t*) malloc(num_threads * sizeof(pthread_t));
    for(int i = 0; i < num_threads; i++) {
        // printf("starting thread %d\n", i);
        pthread_create(&threads[i],NULL,big_sum,&big_sum_args_list[i]);
    }
    for(int i = 0; i < num_threads; i++) {
        pthread_join(threads[i],NULL);
    }
    free(threads);

#else
        for (int i = 0; i < N; i++) {
            add_vector_to_dest(Zsums[features[i].group],features[i].features,f_size);
            Zcounts[features[i].group]++;
        }
#endif


        for (int i = 0; i < k; i++) {
            divide_vector_by_scalar(Z[i],Zsums[i],Zcounts[i],f_size);
        }

        //reset distances to max_val
        memset(D, -1, N * sizeof(uint));

        for (int i = 0; i < N; i++) {
            for (int j = 0; j < k; j++) {
                vector_subtract_abs(temp,features[i].features, Z[j], f_size);
                vector_elementwise_square(temp,temp,f_size);
                temp_d = sum(temp, f_size);
                if(temp_d < D[i]) {
                    D[i] = temp_d;
                    features[i].group = j;
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

    return Z;
}