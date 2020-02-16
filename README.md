# FeiRays
Reusable Vulkan based ray-tracing code base.

The code is evolved from [VkRayTraceWeekend](https://github.com/fynv/VkRayTraceWeekend).

Here, I'm trying to make it more extendable and reusable, so I can have more fun with it.

## Building and Running

Building the project is simple. 

The only dependency you need is CUDA SDK, which is used for initializing the random number generator(RNG) states for each pixel.

[Volk](https://github.com/zeux/volk.git) and [Vulkan-Headers](https://github.com/KhronosGroup/Vulkan-Headers.git) are included as submodules,
so it should be fine to build without Vulkan SDK, and the resulted executables should be adaptive to different Vulkan versions. 
But be sure to have the latest graphics driver installed, one that supports Vulkan. 

* Clone the repo and update the submodules
* Use CMake to generate a VS solution at FeiRays/build (Linux build not tested yet).
* Build and run the tests. 
* Shaders (spv) are expected at ../shaders relative to the starting folder.
* Output file are in 24bit R8G8B8 raw format.

## Progress

In case someone is interested in the progress of the project, here is a list of what have been implemented so far.

All performance numbers are grabbed using a RTX 2060 super.


### Random Number Generator for Shaders

RNGState.h, rand_state_init.cu, shaders/rand.shinc

For monte carlo path-tracing, it is quite essential to have a pseudo-random number generator ready everywhere.
For this purpose, we have a minimal set of XORWOW implementation of CURAND ported here. 
While the initialization of the RNG states are done by CUDA, the recursive random number generation can happen in shaders. 

While the quality of the random number generated from XORWOW is good, the method has the following downsides:

* Initialization is expensive: if the scene is simple, the initialization can cost most of the time
* The size of each RNG state is 6x32bit, which is quite big.

This module can be replaced relatively easily, once we know there is a better method for this purpose.

### Vulkan Based Path-Tracing framework

context.cpp, PathTracer.cpp

There is abstract class called "Geometry". Each sub-class can have its own closest-hit shader and intersection shader (optional).

The class "PathTracer" maintains a list of the geometries and path-traces them.

### rt_weekend

<img src="gallery/rt_weekend.png" width="900px">

	900 x 600 x 100 rays:

	Generating scene..
	Done generating scene.. 0.957384 secs
	Initializing RNG states..
	Done initializing RNG states.. 1.495565 secs
	Preparing ray-tracing..
	Done preparing ray-tracing.. 0.025201 secs
	Doing ray-tracing..
	Done ray-tracing.. 0.193738 secs

This is the classic "Ray Tracing in One Weekend" scene. All 3 kinds of material are implemented.

### test1

<img src="gallery/test1.png" width="800px">

	800 x 400 x 100 rays:

	Initializing RNG states..
	Done initializing RNG states.. 0.527261 secs
	Preparing ray-tracing..
	Done preparing ray-tracing.. 0.025987 secs
	Doing ray-tracing..
	Done ray-tracing.. 0.078368 secs

Besides spheres, triangle-meshes (triangle-lists) are also implemented.

### test2

<img src="gallery/test2.png" width="900px">

	900 x 600 x 100 rays:

	Generating scene..
	Done generating scene.. 0.956308 secs
	Initializing RNG states..
	Done initializing RNG states.. 1.499995 secs
	Preparing ray-tracing..
	Done preparing ray-tracing.. 0.026137 secs
	Doing ray-tracing..
	Done ray-tracing.. 0.194363 secs

Sphere with checker texture. (Chapter 3, Ray Tracing - The Next Week)


### test3

<img src="gallery/test3.png" width="900px">

	900 x 600 x 100 rays:

	Initializing RNG states..
	Done initializing RNG states.. 1.506576 secs
	Preparing ray-tracing..
	Done preparing ray-tracing.. 0.027714 secs
	Doing ray-tracing..
	Done ray-tracing.. 0.156855 secs

Triangle mesh and sphere with diffuse textures.
Sky-box using a cubemap texture.


### test4

<img src="gallery/test4.png" width="900px">

	900 x 600 x 100 rays:

	Initializing RNG states..
	Done initializing RNG states.. 1.546515 secs
	Preparing ray-tracing..
	Done preparing ray-tracing.. 0.027238 secs
	Doing ray-tracing..
	Done ray-tracing.. 0.115665 secs

Test using emissive material.



