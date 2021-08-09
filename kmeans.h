#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <memory.h>
#include <strings.h>

#define k 20


// if source data is images we will likely have 0-255 pixel values
typedef unsigned char uint8;

// poor man's template typing
typedef uint8 feature_type;

typedef unsigned int uint;

struct observation {
    feature_type* features;
    uint group;
};
typedef struct observation observation;


feature_type** kmeans(feature_type** data, int N, int f_size);

observation* kmeans_gpu(observation* data, size_t N, size_t h, size_t w, size_t d);