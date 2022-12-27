
namespace game
{
namespace math
{

#include <math.h> //todo(staffan): get rid of this.
internal inline int ceil_float_to_int(float value)
{
	int result = (int)ceilf(value);
	return result;
}

internal inline int round_float_to_int(float value)
{
	int result = (int)(value+0.5f);
	return result;
}

internal inline uint32_t round_float_to_uint32(float value)
{
	uint32_t result = (uint32_t)(value+0.5f);
	return result;
}

internal inline int floor_float_to_int(float value)
{
	int result = (int)floorf(value);
	return result;
}

internal inline float lerp(float a, float b, float t)
{
	float result = ((1.0f-t)*a) + (t*b);
	return result;
}

}//namespace math
}//namespace game