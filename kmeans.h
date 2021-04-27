#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <memory.h>
#include <strings.h>
#include <stdio.h>

#include "typedefs.h"
#include "save_means.h"


struct feature {
    feature_type* features;
    uint group;
};
typedef struct feature feature;

feature_type** kmeans(feature_type** data, int N, int f_size, int k);