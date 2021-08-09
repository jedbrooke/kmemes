# Kmemes #
sigh, why am I starting another side project when I already have so many that
aren't finished. oh well, here's the plan:
1. ~~steal~~ acquire pictures from reddit/the internet
2. perform kmeans learning on them
3. use the group representatives to generate new images
4. profit!!

Goals:
 - write kmeans, it should be decently fast
 - try to use CUDA? at least multi-threading 
 - probably use c or "c with classes"(c++)


steps:
web scraper to download photos

test algorithm on MNIST hand written digit samples. (it's sort of the "hello world" of ML datasets)

if this goes well we can try learning on memes.

Kmemes isn't really a generative algorithm so checkout meme-gan for my attempt at a GAN.

possibly figure out some kind of optimizations where when we calculate all the
distances for group membership. we can possibly rule out the search area to 
members that we know are certainly not members of a given group.

TODO:
 - [x] make python script to convert mnist data into more workable format
 - [x] make python script to display output mean
 - [ ] implement multithreading
 - [x] implement in CUDA
 - [x] scape memes from reddit
 - [ ] preprocess memes to separate images with a lot of text
 - [ ] work on batch loading system to deal with datasets larger than available RAM

 Timing results: 
Mnist: 60000 samples
| Method | real (s) | user (s) | sys (s) | epochs |
|--------|------|------|-----|-------|
| Python/Numpy<sup>1</sup> | 9,529.991 | 9,522.624 | 9,522.624 | ~64
| Single Threaded C -O0 | 555.550 | 551.086 | 0.967 | 74
| Single Threaded C -03 | 34.905 | 34.730 | 0.056 | 74
| OpenMP |
| Pthread Mulithreaded |
| CUDA<sup>2</sup> | 74.949 | 74.635 | 0.216 | 100
| OpenCL?
| Intel Xeon Phi<sup>3</sup>?

<sup>1</sup>the python version crashed before finishing, I was too lazy to run it again and it's already clear that it is MUCH slower

<sup>2</sup>Running on a Tesla M40 @1.1Ghz. Gpu version uses aboslute difference instead of absolute squared difference, so it converges differently. Final means are very similar, but not identical.
Currently it is quite a bit slower than the CPU version, but hopefully it will scale better with larger images. 
Also the Kernels are optimized for RGB data so it is actually duplicating the BW inputs 3 times to work properly, thus processing 3x the amount of data. So in theory you could divide the time by 3?

<sup>3</sup> yes I have one. Why? for fun.

