#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <memory.h>
#include <strings.h>

#ifdef OMP
    #include <omp.h>
#endif



// if source data is images we will likely have 0-255 pixel values
typedef unsigned char uint8;

// poor man's template typing
typedef uint8 feature_type;

typedef unsigned int uint;

struct feature {
    feature_type* features;
    uint group;
};

typedef struct feature feature;

#ifdef PTHREAD
    #include <pthread.h>

    struct big_sum_args
    {
        int id;
        int stride;
        int f_size;
        int N;
        uint** Zsums;
        feature* features;
        uint* Zcounts;
        pthread_mutex_t* Zsums_locks;

    };
    typedef struct big_sum_args big_sum_args;

    void* big_sum(void* args);
    
#endif

#if defined(OMP) || defined(PTHREAD)
feature_type** kmeans(feature_type** data, int N, int f_size, int k, int num_threads);
#else
feature_type** kmeans(feature_type** data, int N, int f_size, int k);
#endif