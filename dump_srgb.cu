#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

inline __device__ float d_clamp01(float f)
{
	float v = f;
	if (v < 0.0f) v = 0.0f;
	else if (v > 1.0f) v = 1.0f;
	return v;
}

__global__
void g_raw_to_srgb(const float* raw, unsigned char* srgb, size_t num_pixels, float boost)
{
	size_t pix_id = threadIdx.x + blockIdx.x*blockDim.x;
	if (pix_id < num_pixels)
	{
		const float* pIn = raw + pix_id * 4;
		unsigned char* pOut = srgb + pix_id * 3;
		float4 v;
		float power = 1.0f / 2.2f;
		v.x = d_clamp01(powf(pIn[0] * boost, power));
		v.y = d_clamp01(powf(pIn[1] * boost, power));
		v.z = d_clamp01(powf(pIn[2] * boost, power));

		pOut[0] = v.x *255.0f + 0.5f;
		pOut[1] = v.y *255.0f + 0.5f;
		pOut[2] = v.z *255.0f + 0.5f;
	}
}

void h_raw_to_srgb(const float* raw, unsigned char* srgb, size_t num_pixels, float boost)
{
	unsigned num_blocks = (unsigned)((num_pixels + 127) / 128);
	g_raw_to_srgb << <num_blocks, 128 >> > (raw, srgb, num_pixels, boost);
}

