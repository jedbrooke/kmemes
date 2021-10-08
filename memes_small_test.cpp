#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>

#include <stdarg.h>

#define PNG_DEBUG 3
#include <png.h>

#include "kmeans.h"

// png reading code from http://zarb.org/~gc/html/libpng.html
// with modifications

const std::string data_dir("1000memes");
const std::string output_dir("results");
const size_t DATA_H = 512;
const size_t DATA_W = 512;
const size_t DATA_D = 3;
const size_t DATA_BIT_DEPTH = sizeof(feature_type) * 8;
const size_t N = 1000;



void abort_(const char * s, ...) {
        va_list args;
        va_start(args, s);
        vfprintf(stderr, s, args);
        fprintf(stderr, "\n");
        va_end(args);
        abort();
}

void write_png_file(const char* file_name, png_bytepp row_pointers)
{
        /* create file */
        FILE *fp = fopen(file_name, "wb");
        if (!fp)
                abort_("[write_png_file] File %s could not be opened for writing", file_name);


        /* initialize stuff */
        auto png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

        if (!png_ptr)
                abort_("[write_png_file] png_create_write_struct failed");

        auto info_ptr = png_create_info_struct(png_ptr);
        if (!info_ptr)
                abort_("[write_png_file] png_create_info_struct failed");

        if (setjmp(png_jmpbuf(png_ptr)))
                abort_("[write_png_file] Error during init_io");

        png_init_io(png_ptr, fp);


        /* write header */
        if (setjmp(png_jmpbuf(png_ptr)))
                abort_("[write_png_file] Error during writing header");

        png_set_IHDR(png_ptr, info_ptr, DATA_W, DATA_H,
                     DATA_BIT_DEPTH, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        png_write_info(png_ptr, info_ptr);


        /* write bytes */
        if (setjmp(png_jmpbuf(png_ptr)))
                abort_("[write_png_file] Error during writing bytes");

        png_write_image(png_ptr, row_pointers);


        /* end write */
        if (setjmp(png_jmpbuf(png_ptr)))
                abort_("[write_png_file] Error during end of write");

        png_write_end(png_ptr, NULL);

        fclose(fp);
}

observation* read_png_file(const char* file_name, png_bytepp row_pointers) {
    char header[8];    // 8 is the maximum size that can be checked

    /* open file and test for it being a png */
    FILE *fp = fopen(file_name, "rb");
    if (!fp) {
        abort_("[read_png_file] File %s could not be opened for reading", file_name);
    }

    fread(header, 1, 8, fp);
    if (png_sig_cmp((png_const_bytep) header, 0, 8)) {
        abort_("[read_png_file] File %s is not recognized as a PNG file", file_name);
    }

    png_structp png_ptr;
    png_infop info_ptr;


    /* initialize stuff */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr) {
        abort_("[read_png_file] png_create_read_struct failed");
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        abort_("[read_png_file] png_create_info_struct failed");
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        abort_("[read_png_file] Error during init_io");
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    if (png_get_image_width(png_ptr, info_ptr) != DATA_W) {
        printf("image: %s\n",file_name);
        abort_("[read_png_file] unexpected image width %d", png_get_image_width(png_ptr, info_ptr));
    }
    if (png_get_image_height(png_ptr, info_ptr) != DATA_H) {
        abort_("[read_png_file] unexpected image height %d", png_get_image_height(png_ptr, info_ptr));
    }

    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    bool isBW = false;
    switch (color_type) {
        case PNG_COLOR_TYPE_RGB:
            // printf("color type is RGB\n");
            break;
        case PNG_COLOR_TYPE_GRAY:
            // printf("color type is grey\n");
            isBW = true;
            break;
        case PNG_COLOR_TYPE_PALETTE:
            // printf("color type is paletted, converting to RGB\n");
            png_set_palette_to_rgb(png_ptr);
            break;
        default:
            printf("on file %s:\n",file_name);
            abort_("Expected color type %d, got %d\n",PNG_COLOR_TYPE_RGB,color_type);
    }

    if(png_get_bit_depth(png_ptr, info_ptr) != DATA_BIT_DEPTH) {
        // printf("on file %s:\n",file_name);
        // std::cout << "num bytes " << png_get_rowbytes(png_ptr,info_ptr) << std::endl;
        // abort_("bit_depth must be %d, got %d\n", DATA_BIT_DEPTH, png_get_bit_depth(png_ptr, info_ptr));
        // TODO: handle bit depths other than 8
        return NULL;
    }

    size_t number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);


    /* read file */
    if (setjmp(png_jmpbuf(png_ptr))) {
        printf("file %s\n",file_name);
        std::cout << "num bytes " << png_get_rowbytes(png_ptr,info_ptr) << std::endl;
        abort_("[read_png_file] Error during read_image");
    }
    png_read_image(png_ptr, row_pointers); 
    // std::cout << "reading image" << std::endl;
    observation* o = (observation*) malloc(sizeof(observation));
    o->features = (feature_type*) malloc(DATA_H * DATA_W * DATA_D * sizeof(feature_type));
    for(size_t i = 0; i < DATA_H; i++) {
        if(isBW) {
            for (size_t x = 0; x < DATA_W; x++) {
                memset(&o->features[i * DATA_W * DATA_D + (x * DATA_D)],row_pointers[i][x],DATA_D * sizeof(feature_type));
            }   
        } else {
            memcpy(&o->features[i * DATA_W * DATA_D], row_pointers[i], DATA_W * DATA_D * sizeof(feature_type));
        }
    }
    fclose(fp);
    return o;
}

int main(int argc, char const *argv[])
{
    png_bytepp row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * DATA_H);
    for (size_t y = 0; y < DATA_H; y++) {
        row_pointers[y] = (png_byte*) malloc(DATA_W * DATA_D * sizeof(png_byte));
    }

    observation observations[N];
    size_t counter = 0;
    for(const auto & file: std::filesystem::directory_iterator(data_dir)) {
        observation* o = read_png_file(std::filesystem::absolute(file.path()).c_str(), row_pointers);
        if(o != NULL) {
            observations[counter] = *o;
            counter++;
        }
        if(counter >= N) {
            break;
        }
    }
    std::cout << "read " << counter << " images" << std::endl;
    observation* means = kmeans_gpu(observations, counter, DATA_H, DATA_W, DATA_D);
    // observation* means = kmeans(observations, counter, DATA_H * DATA_W * DATA_D);
    for(size_t i = 0; i < k; i++) {
        // TODO: multithreaded image loading
        std::string path = output_dir + "/mean_" + std::string(3 - std::to_string(i).length(),'0') + std::to_string(i) + ".png";
        for(size_t y = 0; y < DATA_H; y++) {
            memcpy(row_pointers[y], &means[i].features[y * DATA_W * DATA_D], DATA_W * DATA_D * sizeof(feature_type));
        }
        write_png_file(path.c_str(), row_pointers);
    }

     /* cleanup heap allocation */
    for (size_t y=0; y < DATA_H; y++)
            free(row_pointers[y]);
    free(row_pointers);


    return 0;
}

