#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "RNGState_simple.h"

__global__
void g_rand_init(RNGState* d_states, unsigned width, unsigned height)
{
	unsigned x = threadIdx.x + blockIdx.x*blockDim.x;
	unsigned y = threadIdx.y + blockIdx.y*blockDim.y;
	if (x >= width || y>=height) return;
	unsigned v0 = x, v1 = y, s0 = 0;
	for (unsigned n = 0; n < 16; n++)
	{
		s0 += 0x9e3779b9;
		v0 += ((v1 << 4) + 0xa341316c) ^ (v1 + s0) ^ ((v1 >> 5) + 0xc8013ea4);
		v1 += ((v0 << 4) + 0xad90777d) ^ (v0 + s0) ^ ((v0 >> 5) + 0x7e95761e);
	}
	d_states[x + y * width] = v0;
}


void cu_rand_init(unsigned width, unsigned height, RNGState* d_states)
{
	dim3 dimBlock, dimGrid;
	dimBlock.x = 8;
	dimBlock.y = 8;
	dimGrid.x = (width + 7) / 8;
	dimGrid.y = (height + 7) / 8;
	g_rand_init << < dimBlock, dimGrid >> > ( d_states, width, height);
}

void h_rand_init(unsigned width, unsigned height, RNGState* h_states)
{
	RNGState* d_states;
	cudaMalloc(&d_states, sizeof(RNGState)* width*height);

	cu_rand_init(width, height, d_states);

	cudaMemcpy(h_states, d_states, sizeof(RNGState)* width*height, cudaMemcpyDeviceToHost);
	cudaFree(d_states);
}

