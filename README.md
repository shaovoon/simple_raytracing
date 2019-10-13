# simple_raytracing

The (free) ebook and code are from [Raytracing in a weekend Github](https://github.com/RayTracing/InOneWeekend), amended with some changes to compile in VC++ to run on Windows.

Mainly, drand48() on Unix/Linux is replaced with C++11 random number generator. The code is changed from single-threaded to multi-threaded using Microsoft PPL.

The result image is kinda fuzzy. To fix the fuzziness, try increase the sampling count from 10 to 1000. Higher the better, but it increases the ray-tracing time. Sampling count is held in the ns variable.
