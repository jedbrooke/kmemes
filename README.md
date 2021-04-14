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

pre-process photos for learning.

likely just use image-magick convert to convert them to bmp, resize if needed,
then read into memory from there.

start with 400x400 to reduce memory/time requirements
or start with mnist database for initial algorithm, or the faces dataset
experiment with augemnting data with simple transforms, scale, rotate,
slight color adjust, etc.


if this goes well we can try learning on videos

just make each vector the flattened frame sequence of a video, with the audio 
data in there too

the temporal aspect of video might be too much for kmeans to learn, might need
more advanced techniques.

still it could be really fun.

Is kmeans really meant for generating though? or is it more for unsupervised
classification. Perhpas we need something more like PCA?

possibly figure out some kind of optimizations where when we calculate all the
distances for group membership. we can possibly rule out the search area to 
members that we know are certainly not members of a given group.

TODO:
 - [x] make python script to convert mnist data into more workable format
 - [x] make python script to display output mean
 - [] implement multithreading
 - [] implement in CUDA

 Timing results: 
Mnist: 60000 samples
| Method | real (s) | user (s) | sys (s) | epochs |
|--------|------|------|-----|-------|
| Python/Numpy* | 9,529.991 | 9,522.624 | 9,522.624 | ~64
| Single Threaded C -O0 | 555.550 | 551.086 | 0.967 | 74
| Single Threaded C -03 | 34.905 | 34.730 | 0.056 | 74
| Pthread Mulithreaded N=6
| Pthread Multithreaded N=12
| CUDA
| OpenCL?
| Phi/OpenMP?

*the python version crashed before finishing, I was too lazy to run it again and it's already clear that it is MUCH slower
