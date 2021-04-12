#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <memory.h>

// if source data is images we will likely have 0-255 pixel values
typedef unsigned char uint8;

// poor man's template typing
typedef uint8 feature_type;

typedef unsigned int uint;

struct feature {
    feature_type* features;
    int group;
};
typedef struct feature feature;

feature_type** kmeans(feature_type** data, int N, int f_size, int k);