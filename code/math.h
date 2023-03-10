namespace math
{

#include <math.h> //todo(staffan): get rid of this.
internal inline int ceil_float_to_int(float32 value)
{
	int result = (int)ceilf(value);
	return result;
}

internal inline int round_float_to_int(float32 value)
{
	int result = (int)roundf(value);
	return result;
}

internal inline uint32 round_float_to_uint32(float32 value)
{
	uint32 result = (uint32)(value+0.5f);
	return result;
}

internal inline int floor_float_to_int(float32 value)
{
	int result = (int)floorf(value);
	return result;
}

internal inline float32 lerp(float32 value1, float32 value2, float32 t)
{
	float32 result = ((1.0f-t)*value1) + (t*value2);
	return result;
}

internal inline
float32 square_root(float32 value)
{
	float32 result;
	result = (float32)sqrt(value);
	return result;
}

internal inline
int32 sign(float32 value)
{
	int32 result;
	result = signbit(value);
	return result;
}

}//namespace math

#include "vector2.h"