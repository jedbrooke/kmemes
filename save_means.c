#include "save_means.h"

int current_frame = 0;
char* base_path = "means/mean_";
char* ext = "csv";

bool save_means(feature_type** means, uint k, uint f_size) {
    char path[128];
    sprintf(path,"%s%05d.%s",base_path,current_frame,ext);
    
    FILE* output_fp = fopen(path,"w");
    if(!output_fp) {
        perror("fopen");
        return false;
    }
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < f_size; j++) {
            fprintf(output_fp,"%u,",means[i][j]);
        }
        fprintf(output_fp,"\n");
    }
    fflush(output_fp);
    fclose(output_fp);
    
    current_frame++;
    return true;
}