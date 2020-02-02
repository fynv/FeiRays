#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "xor_wow_data.hpp"
#include "RNGState.h"

struct RNG
{
	unsigned* d_sequence_matrix;
	unsigned* d_offset_matrix;

	__device__ inline void state_init(unsigned long long seed,
		unsigned long long subsequence,
		unsigned long long offset,
		RNGState& state)
	{
		unsigned int s0 = ((unsigned int)seed) ^ 0xaad26b49UL;
		unsigned int s1 = (unsigned int)(seed >> 32) ^ 0xf7dcefddUL;
		unsigned int t0 = 1099087573UL * s0;
		unsigned int t1 = 2591861531UL * s1;
		state.d = 6615241 + t1 + t0;
		state.v.v0 = 123456789UL + t0;
		state.v.v1 = 362436069UL ^ t0;
		state.v.v2 = 521288629UL + t1;
		state.v.v3 = 88675123UL ^ t1;
		state.v.v4 = 5783321UL + t0;

		// apply sequence matrix
		V5 result;
		unsigned long long p = subsequence;
		int i_mat = 0;
		unsigned matrix[800];
		unsigned matrixA[800];

		while (p && i_mat < 7)
		{
			for (unsigned int t = 0; t < (p & 3); t++)
			{
				matvec(state.v, d_sequence_matrix + i_mat * 800, result);
				state.v = result;
			}
			p >>= 2;
			i_mat++;
		}
		if (p)
		{
			memcpy(matrix, d_sequence_matrix + i_mat * 800, sizeof(unsigned) * 800);
			memcpy(matrixA, d_sequence_matrix + i_mat * 800, sizeof(unsigned) * 800);
		}

		while (p)
		{
			for (unsigned int t = 0; t < (p & 0xF); t++)
			{
				matvec(state.v, matrixA, result);
				state.v = result;
			}
			p >>= 4;
			if (p)
			{
				for (int i = 0; i < 4; i++)
				{
					matmat(matrix, matrixA);
					memcpy(matrixA, matrix, sizeof(unsigned) * 800);
				}
			}
		}

		// apply offset matrix
		p = offset;
		i_mat = 0;
		while (p && i_mat < 7)
		{
			for (unsigned int t = 0; t < (p & 3); t++)
			{
				matvec(state.v, d_offset_matrix + i_mat * 800, result);
				state.v = result;
			}
			p >>= 2;
			i_mat++;
		}

		if (p)
		{
			memcpy(matrix, d_offset_matrix + i_mat * 800, sizeof(unsigned) * 800);
			memcpy(matrixA, d_offset_matrix + i_mat * 800, sizeof(unsigned) * 800);
		}

		while (p)
		{

			for (unsigned int t = 0; t < (p & 0xF); t++)
			{
				matvec(state.v, matrixA, result);
				state.v = result;
			}
			p >>= 4;
			if (p)
			{
				for (int i = 0; i < 4; i++)
				{
					matmat(matrix, matrixA);
					memcpy(matrixA, matrix, sizeof(unsigned) * 800);
				}
			}
		}
		state.d += 362437 * (unsigned int)offset;
	}

private:
	static __device__ inline void matvec_i(int i, unsigned v_i, const unsigned *matrix, V5& result)
	{
		for (int j = 0; j < 32; j++)
			if (v_i & (1 << j))
			{
				V5 mat_row = ((V5*)matrix)[i * 32 + j];
				result.v0 ^= mat_row.v0;
				result.v1 ^= mat_row.v1;
				result.v2 ^= mat_row.v2;
				result.v3 ^= mat_row.v3;
				result.v4 ^= mat_row.v4;
			}
	}

	static __device__ inline void matvec(const V5& vector, const unsigned *matrix, V5& result)
	{
		memset(&result, 0, sizeof(V5));
		matvec_i(0, vector.v0, matrix, result);
		matvec_i(1, vector.v1, matrix, result);
		matvec_i(2, vector.v2, matrix, result);
		matvec_i(3, vector.v3, matrix, result);
		matvec_i(4, vector.v4, matrix, result);
	}

	static __device__ inline void matmat(unsigned int *matrixA, const unsigned int *matrixB)
	{
		V5 result;
		for (int i = 0; i < 160; i++)
		{
			matvec(((V5*)matrixA)[i], matrixB, result);
			((V5*)matrixA)[i] = result;
		}
	}
};

__global__
void g_rand_init(RNG rng, RNGState* d_states, unsigned count)
{
	unsigned id = threadIdx.x + blockIdx.x*blockDim.x;
	if (id >= count) return;
	rng.state_init(1234, id, 0, d_states[id]);
}

void cu_rand_init(unsigned count, RNGState* d_states)
{
	RNG rng;
	cudaMalloc(&rng.d_sequence_matrix, sizeof(unsigned) * 800 * 8);
	cudaMalloc(&rng.d_offset_matrix, sizeof(unsigned) * 800 * 8);
	cudaMemcpy(rng.d_sequence_matrix, xorwow_sequence_matrix, sizeof(unsigned) * 800 * 8, cudaMemcpyHostToDevice);
	cudaMemcpy(rng.d_offset_matrix, xorwow_offset_matrix, sizeof(unsigned) * 800 * 8, cudaMemcpyHostToDevice);

	unsigned blocks = (count + 127) / 128;
	g_rand_init << < blocks, 128 >> > (rng, d_states, count);

	cudaFree(rng.d_offset_matrix);
	cudaFree(rng.d_sequence_matrix);

}

void h_rand_init(unsigned count, RNGState* h_states)
{

	RNGState* d_states;
	cudaMalloc(&d_states, sizeof(RNGState)* count);	

	cu_rand_init(count, d_states);

	cudaMemcpy(h_states, d_states, sizeof(RNGState)* count, cudaMemcpyDeviceToHost);
	cudaFree(d_states);
}
