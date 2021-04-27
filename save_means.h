#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <string.h>

#include "typedefs.h"

bool save_means(feature_type** means, uint k, uint f_size);