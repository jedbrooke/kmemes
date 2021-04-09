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
