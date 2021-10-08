#include "kmeans.h"
#include <cstdio>
#include <device_launch_parameters.h>




uint sum(uint* a, uint n){
    uint sum = 0;
    for (uint i = 0; i < n; i++){
        sum += a[i];
    }
    return sum;
}


#define CUDA_CALL(x) do {if((x)!=cudaSuccess) { \
    printf("Error at %s:%d\n",__FILE__,__LINE__);}} while(0)

// Zsums is k * w * h * d, the sum of all observations in a group
// Z is Zsum/l, the average of all observations in a group
// counts is the number of observations in a group
// k is the number of groups

__global__
void divide_Zsums(const uint* Zsums, feature_type* Z, const uint* counts) {
    const size_t w = blockDim.x * gridDim.x;
    const size_t h = blockDim.y * gridDim.y;
    const size_t d = blockDim.z;
    const size_t posx = blockIdx.x * blockDim.x + threadIdx.x;
    const size_t posy = blockIdx.y * blockDim.y + threadIdx.y;
    
    __shared__ size_t counts_shared[k];
    
    // the 0th thread in each block loads the shared mem.
    // TODO: see if this is even faster than just not using shared mem.
    if (threadIdx.x == 0 and threadIdx.y == 0 and threadIdx.z == 0) {
#pragma unroll
        for(size_t i = 0; i < k; i++){
            counts_shared[i] = counts[i];
        }
    }
    __syncthreads();

    for(size_t i = 0; i < k; i++) {
        size_t idx = i * (w * h * d) + (posy * w * d) + (posx * d) + threadIdx.z;
        Z[idx] = Zsums[idx] / counts_shared[i];
    }

}

// Z is k * h * w * d
// Z holds the group representatives
// observations is N * h * w * d
// group_distances is N * k, distance to each group for each observation
// h,w,d are the dimensions of each observation
// d is assumed to be 3

// each block handles observation at a time
// each block is k * rowSize
// each thread handles 1 feature and 1 group


__global__
void calculate_group_distances(const unsigned char* Z, const unsigned char* observations, uint* group_distances, const size_t h, const size_t w, const size_t N) {
    const size_t index = (blockIdx.z * gridDim.y * gridDim.x) + (blockIdx.y * gridDim.x) + blockIdx.x;
    const size_t stride = gridDim.x * gridDim.y * gridDim.z;
    
    for(size_t i = index; i < N; i+= stride) {
        __shared__ uint distances[k][ROW_SIZE];

        distances[threadIdx.x][threadIdx.y] = 0;

        const size_t observation_offset = i * h * w * 3;
        
        for(size_t posy = 0; posy < h; posy++) {
            for(size_t posx = 0; posx < w; posx += ROW_SIZE) {
                
                __shared__ u_int32_t row[ROW_SIZE];
                if(threadIdx.x == 0) {
                    const size_t pixel_offset = observation_offset + (posy * w * 3) + ((posx + threadIdx.y) * 3);
                    row[threadIdx.y] = (0x0000) | (observations[pixel_offset + 0] << 16)
                                                | (observations[pixel_offset + 1] <<  8)
                                                | (observations[pixel_offset + 2] <<  0);
                }

                __syncthreads();
                const size_t pixel_offset = (threadIdx.x * h * w * 3) + (posy * w) + ((posx + threadIdx.y) * 3);
                u_int32_t group_rep = (0x0000)  | (Z[pixel_offset + 0] << 16)
                                                | (Z[pixel_offset + 1] <<  8)
                                                | (Z[pixel_offset + 2] <<  0);
                // Sum(|Z[i] - Observation[i]|)
                // original algo squares each difference, but this should work too, might just converge slower
                distances[threadIdx.x][threadIdx.y] += __vsadu4(group_rep, row[threadIdx.y]);
            }
        }
        __syncthreads();
        if(threadIdx.y == 0) {
            uint distance = 0;
            for(size_t group = 0; group < k; group++) {
                distance += distances[threadIdx.x][group];
            }
            group_distances[(i*k) + threadIdx.x] = distance;
        }
    }
}

// f = h * w * d, the number of features in each observation
// Z is k * f
// Z holds the group representatives
// observations is N * f
// group_distances is N * k, distance to each group for each observation
// h,w,d are the dimensions of each observation

// each block handles 1 observation
// each block handles observation at a time
// each block is k * ROW_SIZE
// each thread handles 1 feature and 1 group

__global__
void calculate_group_squared_distances(const unsigned char* Z, const unsigned char* observations, uint* group_distances, const size_t f, const size_t N) {
    const size_t index = (blockIdx.z * gridDim.y * gridDim.x) + (blockIdx.y * gridDim.x) + blockIdx.x;
    const size_t stride = gridDim.x * gridDim.y * gridDim.z;

    for(size_t i = index; i < N; i+= stride) {
        
        __shared__ uint distances[k][ROW_SIZE];
        distances[threadIdx.x][threadIdx.y] = 0;
        
        const size_t observation_offset = i * f;
        
        for(size_t j = 0; j < f; j+= ROW_SIZE) {
            __shared__ feature_type chunk[ROW_SIZE];
            if(threadIdx.x == 0) {
                chunk[threadIdx.y] = observations[observation_offset + j + threadIdx.y];
            }
            __syncthreads();
            // int temp = Z[threadIdx.x * j] - chunk[threadIdx.y];
            distances[threadIdx.x][threadIdx.y] += abs(Z[threadIdx.x * j] - chunk[threadIdx.y]);

        }
        __syncthreads();
        if(threadIdx.y == 0) {
            uint distance = 0;
            for(size_t group = 0; group < k; group++) {
                distance += distances[threadIdx.x][group];
            }
            group_distances[(i * k) + threadIdx.x] = distance;
        } 
    }

}
// group_distances is N * k
// min_distances is N *
// groups is N *
// each thread handles 1 observation

__global__
void find_min_distance_and_group(const uint* group_distances, uint* min_distances, uint* groups, size_t N) {
    const size_t index = (blockIdx.x * blockDim.x) + threadIdx.x;
    const size_t stride = blockDim.x * gridDim.x;
    for(size_t i = index; i < N; i+=stride) {
        uint min_distance = INT_MAX;
        int group = -1;
        for(size_t j = 0; j < k; j++) {
            if (group_distances[(i * k) + j] < min_distance) {
                min_distance = group_distances[(i * k) + j];
                group = j;
            }
        }   
        min_distances[i] = min_distance;
        groups[i] = group;
        
    }

}


// each thread will handle a single feature location across all observations
// each block is BLOCK_SIZExBLOCK_SIZEx3
// each x,y in a block will handle an x,y in the image
// and each thread along z will handle r,g,b respectively


// Zsum is h * w * d
// features is N * h * w * d
// groups is N, hold the group number of each observation

// goup is the target group we are summing for
// N is the number of observations

__global__
void calc_Zsums_for_group(const feature_type* observations, uint* groups, uint* Zsums, size_t group, size_t N) {
    const size_t w = blockDim.x * gridDim.x;
    const size_t h = blockDim.y * gridDim.y;
    const size_t d = blockDim.z;
    const size_t posx = (blockIdx.x * blockDim.x) + threadIdx.x;
    const size_t posy = (blockIdx.y * blockDim.y) + threadIdx.y;
    const size_t group_offset = group * h * w * d;
    for(size_t i = 0; i < N; i++) {
        if(groups[i] == group) {
            const size_t feature_idx = (posy * w * d) + (posx * d) + threadIdx.z;
            const size_t observation_idx = i * (h * w * d) + feature_idx;
            Zsums[group_offset + feature_idx] += observations[observation_idx];
        }
    }

}

__global__
void print_arr(uint* arr, size_t N) {
    if(threadIdx.x == 0 and blockIdx.x == 0) {
        for(size_t i = 0; i < N; i++) {
            printf("%u,",arr[i]);
        }
        printf("\n");
    }
}

observation* kmeans_gpu(observation* data, size_t N, size_t h, size_t w, size_t d) {
    
    // allocate memory for group representatives
    feature_type* Z_cu;
    CUDA_CALL(cudaMalloc(&Z_cu, k * h * w * d * sizeof(feature_type)));

    // allocate memory for temporary group sums
    uint* Z_sums_cu;
    CUDA_CALL(cudaMalloc(&Z_sums_cu, k * h * w * d * sizeof(uint)));

    // allocate memory to store group counts
    uint* Zcounts = (uint*) malloc(k * sizeof(uint));
    uint* Zcounts_cu;
    CUDA_CALL(cudaMalloc(&Zcounts_cu, k * sizeof(uint)));


    // allocate memory on gpu to hold observation data
    feature_type* observations_cu;
    CUDA_CALL(cudaMalloc(& observations_cu, N * h * w * d * sizeof(feature_type)));

    // allocate memory to store group distances
    uint* group_distances_cu;
    CUDA_CALL(cudaMalloc(&group_distances_cu, N * k * sizeof(uint)));

    // allocate memory for minimum distances
    uint* min_distances_cu;
    CUDA_CALL(cudaMalloc(&min_distances_cu, N * sizeof(uint)));
    uint* D = (uint*) malloc(N * sizeof(uint));
    
    // send data to gpu
    // TODO: split this up into batches of data, since we can't fit all of it in VRAM
    const size_t size = (h * w * d * sizeof(feature_type));
    for(size_t i = 0; i < N; i++) {
        CUDA_CALL(cudaMemcpy(&observations_cu[(i * h * w * d)], data[i].features, size, cudaMemcpyHostToDevice));
    }
    CUDA_CALL(cudaDeviceSynchronize());


    float prev_j = 0;
    float Jscore = -1;
    float threshold = 1e-4;

    bool stop_looping = false;

    int max_iterations = 100;
    int iterations = 0;

    // assign initial random groups
    for(size_t i = 0; i < N; i++) {
        // non-random for now so we can get consistent results during testing
        data[i].group = i % k;
    }

    // send group assignments to GPU
    uint* groups = (uint*) malloc(N * sizeof(uint));
    uint* groups_cu;
    CUDA_CALL(cudaMalloc(&groups_cu, N * sizeof(uint)));
    for(size_t i = 0; i < N; i++) {
        groups[i] = data[i].group;
    }
    CUDA_CALL(cudaMemcpy(groups_cu, groups, N * sizeof(uint), cudaMemcpyHostToDevice));

    // standard 1 thread per pixel component block/grid size used by many kernels
    dim3 standardBlock(BLOCK_SIZE,BLOCK_SIZE,d);
    printf("standard block: %d,%d,%d\n",standardBlock.x, standardBlock.y, standardBlock.z);
    dim3 standardGrid(h / BLOCK_SIZE, w / BLOCK_SIZE);
    printf("standard grid: %d,%d,%d\n",standardGrid.x, standardGrid.y, standardGrid.z);


    // k * ROW_SIZE blocks for group distance kernel
    dim3 rowBlock(k,ROW_SIZE);
    printf("row block grid: %d,%d,%d\n",rowBlock.x, rowBlock.y, rowBlock.z);

    // get num SMs, used for launching some kernels
    int numSMs;
    CUDA_CALL(cudaDeviceGetAttribute(&numSMs, cudaDevAttrMultiProcessorCount, 0));
    

    while(not stop_looping) {
        printf("epoch %d\n", iterations);

        CUDA_CALL(cudaMemset(Z_sums_cu, 0, k * h * w * d * sizeof(uint)));
        bzero(Zcounts, k * sizeof(uint));


        // sum all the members in each group to find the averages
        // TODO: split this up into batches of data, since we can't fit all of it in VRAM

        // TODO: can we speed this up by running each k in a separate stream?
        for(size_t g = 0; g < k; g++) {
            calc_Zsums_for_group<<<standardGrid,standardBlock>>>(observations_cu, groups_cu, Z_sums_cu, g, N);
            cudaDeviceSynchronize();
        }
        // printf("Zsums on gpu\n");
        // print_arr<<<1,1>>>(Z_sums_cu, k);
        
        // calculate the counts for each group
        for(size_t i = 0; i < N; i++) {
            Zcounts[groups[i]] += 1;
        }

        // printf("Zcounts:\n");
        // for(size_t i = 0; i < k; i++) {
        //     printf("%d,",Zcounts[i]);
        // }
        // printf("\n");

        // send Zcounts to the gpu
        CUDA_CALL(cudaMemcpy(Zcounts_cu, Zcounts, k * sizeof(uint), cudaMemcpyHostToDevice));
        cudaDeviceSynchronize();
        
        printf("Zcounts on gpu\n");
        print_arr<<<1,1>>>(Zcounts_cu, k);

        divide_Zsums<<<standardGrid,standardBlock>>>(Z_sums_cu, Z_cu, Zcounts_cu);
        cudaDeviceSynchronize();

        // calculate group distances
        calculate_group_distances<<<32*numSMs,rowBlock>>>(Z_cu, observations_cu, group_distances_cu, h, w, N);
        // calculate_group_squared_distances<<<32*numSMs,rowBlock>>>(Z_cu, observations_cu, group_distances_cu, h * w * d, N);
        cudaDeviceSynchronize();

        // find minimum distance for each observation and assign groups
        find_min_distance_and_group<<<32*numSMs, BLOCK_SIZE * BLOCK_SIZE>>>(group_distances_cu, min_distances_cu, groups_cu, N);
        cudaDeviceSynchronize();

        // copy groups back to cpu
        CUDA_CALL(cudaMemcpy(groups, groups_cu, N * sizeof(uint), cudaMemcpyDeviceToHost));
        cudaDeviceSynchronize();

        // copy min distances back to host
        // TODO: since D is already in gpu mem, and it may be big enough to benefit from a parallel reduction sum
        // we could just calculate the Jscore on the gpu
        cudaMemcpy(D, min_distances_cu, N * sizeof(uint), cudaMemcpyDeviceToHost);
        cudaDeviceSynchronize();

        Jscore = abs((float) sum(D,N) / N);
        printf("J = %0.5f\n",Jscore);
        printf("Diff = %0.5f\n",fabs(Jscore - prev_j));
        printf("Thresh = %0.5f\n",(threshold * Jscore));
        iterations++;
        stop_looping = fabs(Jscore - prev_j) < fabs(threshold * Jscore) || iterations >= max_iterations;
        prev_j = Jscore;

    }

    observation* Z = (observation*) malloc(k * sizeof(observation));
    for(size_t i = 0; i < k; i++) {
        Z[i].features = (feature_type*) malloc(h * w * d * sizeof(feature_type));
        CUDA_CALL(cudaMemcpyAsync(Z[i].features,&Z_cu[i * h * w * d], h * w * d * sizeof(feature_type), cudaMemcpyDeviceToHost));
        Z[i].group = i;
    }
    cudaDeviceSynchronize();
    return Z;
}