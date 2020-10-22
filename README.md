# simple_raytracing

The (free) ebook and code are from [Raytracing in a weekend Github](https://github.com/RayTracing/raytracing.github.io), amended with some changes to compile in VC++ to run on Windows.

Mainly, drand48() on Unix/Linux is replaced with C++11 random number generator. The code is changed from single-threaded to multi-threaded.

The improvements I made to the original code:

* Convert all raw allocation to shared_ptr to prevent memory leaks.
* Replace non-reentrant rand() with a copy of C++11 random number generator in every thread using thread_local to avoid sharing and locking.
* Replace the text-based image format with stb_image saving to PNG.
* Parallelize with C++17 parallel for_each, achieving a more than 5X performance on my 6 core CPU.

The result image is kinda fuzzy. To fix the fuzziness, try increase the sampling count from 10 to 500. Higher the better, but it increases the ray-tracing time. Sampling count is held in the ns variable.
