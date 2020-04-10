#include <vector>
#include "RNGState_xorwow.h"
#include "RNGInitializer.h"
#include "Timing.h"

int main()
{
	size_t count = 1 << 18;
	std::vector<RNGState> states(count);
	double time0 = GetTime();

	DeviceBuffer d_states(sizeof(RNGState)*count, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
	const RNGInitializer& initializer = RNGInitializer::get_initializer();
	initializer.InitRNGs(&d_states);
	d_states.download(states.data());

	double time1 = GetTime();
	printf("time: %f\n", time1 - time0);
	FILE* fp = fopen("dump_rnd", "w");
	for (int i = 0; i < count; i++)
	{
		fprintf(fp, "%u %u %u %u %u %u\n", states[i].v.v0, states[i].v.v1, states[i].v.v2, states[i].v.v3, states[i].v.v4, states[i].d);
	}
	fclose(fp);

	
	return 0;
}

