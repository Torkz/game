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

internal inline
float32 min(float32 a, float32 b)
{
	float32 result = (a<b) ? a : b;
	return result;
}

internal inline
float32 max(float32 a, float32 b)
{
	float32 result = (a>b) ? a : b;
	return result;
}

internal inline
uint32 min(uint32 a, uint32 b)
{
	uint32 result = (a<b) ? a : b;
	return result;
}

internal inline
uint32 max(uint32 a, uint32 b)
{
	uint32 result = (a>b) ? a : b;
	return result;
}

internal inline
uint16 min(uint16 a, uint16 b)
{
	uint16 result = (a<b) ? a : b;
	return result;
}

internal inline
uint16 max(uint16 a, uint16 b)
{
	uint16 result = (a>b) ? a : b;
	return result;
}

}//namespace math

#include "vector2.h"

namespace math
{

struct line_intersection
{
	bool found;
	bool collinear;
	vector2 position;
	vector2 position2;
};

internal inline
line_intersection line_vs_line_intersection(vector2 a, vector2 b, vector2 c, vector2 d)
{
	//https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect#
	line_intersection result;

	vector2 p = a;
	vector2 r = b-a;
	vector2 q = c;
	vector2 s = d-c;

	vector2 pq = q-p;

	float32 r_s_cross = cross(r, s);
	float32 pq_s_cross = cross(pq, s);
	float32 pq_r_cross = cross(pq, r);

	float32 t = pq_s_cross / r_s_cross;
	float32 u = pq_r_cross / r_s_cross;

	if(r_s_cross == 0.0f && pq_r_cross == 0.0f)
	{
		//lines are collinear, check for overlap
		float32 rr_dot = dot(r, r);
		float32 sr_dot = dot(s, r);

		float32 t0 = dot(pq, r) / rr_dot;
		float32 t1 = t0 + sr_dot / rr_dot;

		if(sr_dot < 0.0f && t1 <= 0.0f && t0 >= 1.0f)
		{
			result.found = true;
			result.collinear = true;
			result.position = {max(a.x, c.x), max(a.y, c.y)};
			result.position2 = {min(b.x, d.x), min(b.y, d.y)};
		}
		else if(t0 <= 0.0f && t1 >= 1.0f)
		{
			result.found = true;
			result.collinear = true;
			result.position = {max(a.x, c.x), max(a.y, c.y)};
			result.position2 = {min(b.x, d.x), min(b.y, d.y)};
		}
		else
		{
			result.found = false;
		}
	}
	else if(r_s_cross != 0.0f && (t >= 0.0f && t<=1.0f && u >= 0.0f && u <= 1.0f))
	{
		result.found = true;
		result.position = p + t*r;
	}
	else
	{
		result.found = false;
	}

	return result;
}

internal inline
bool aabb_vs_aabb_overlap(vector2 a_min, vector2 a_max, vector2 b_min, vector2 b_max)
{
	bool result;
	
	if(a_min.x + (a_max.x-a_min.x) < b_min.x
	|| b_min.x + (b_max.x-b_min.x) < a_min.x)
	{
		result = false;
		return result;
	}
	
	if(a_min.y + (a_max.y-a_min.y) < b_min.y
	|| b_min.y + (b_max.y-b_min.y) < a_min.y)
	{
		result = false;
		return result;
	}

	result = true;
	return result;
}

struct aabb_overlap_area_result
{
	vector2 min;
	vector2 max;
};
internal inline
aabb_overlap_area_result aabb_vs_aabb_overlap_area(vector2 a_min, vector2 a_max, vector2 b_min, vector2 b_max)
{
	//note(staffan): assumes there is an overlap
	aabb_overlap_area_result result;
	result.min = {max(a_min.x, b_min.x), max(a_min.y, b_min.y)};
	result.max = {min(a_max.x, b_max.x), min(a_max.y, b_max.y)};
	return result;
}

enum shape_type
{
	aabb = 0
};

struct shape
{
	shape_type type;
	vector2 points[4];
};

internal inline
bool aabb_vs_aabb_overlap(shape aabb_1, shape aabb_2)
{
	bool result;
	vector2 min_1, max_1, min_2, max_2;
	min_1 = aabb_1.points[0];
	max_1 = aabb_1.points[2];
	min_2 = aabb_2.points[0];
	max_2 = aabb_2.points[2];
	result = aabb_vs_aabb_overlap(min_1, max_1, min_2, max_2);
	return result;
}

struct sweep_result
{
	bool collision;
	vector2 position;
	vector2 normal;
	float32 distance_on_sweep_vector;
};
internal inline
sweep_result aabb_vs_aabb_sweep(shape aabb_1, shape aabb_2, vector2 sweep_vector)
{
	sweep_result result;

	vector2 min_1, max_1, min_2, max_2;
	min_1 = aabb_1.points[0];
	max_1 = aabb_1.points[2];
	min_2 = aabb_2.points[0];
	max_2 = aabb_2.points[2];
	result.collision = aabb_vs_aabb_overlap(min_1, max_1, min_2, max_2);
	if(!result.collision)
	{
		return result;
	}

	aabb_overlap_area_result overlap_area = aabb_vs_aabb_overlap_area(min_1, max_1, min_2, max_2);

	const_exp math::vector2 right = {1.0f, 0.0f};
	const_exp math::vector2 up = {0.0f, 1.0f};
	const_exp math::vector2 left = {-1.0f, 0.0f};
	const_exp math::vector2 down = {0.0f, -1.0f};
	
	math::vector2 a, b, c, d, e, normal_1, normal_2;

	float32 right_dot = math::dot(sweep_vector, right);
	float32 up_dot = math::dot(sweep_vector, up);

	if(right_dot > 0.0f) //right
	{
		if(up_dot > 0.0f) //up
		{
			a = overlap_area.max;
			c = {min_2.x, max_2.y};
			d = {min_2.x, min_2.y};
			e = {max_2.x, min_2.y};
			normal_1 = left;
			normal_2 = down;
		}
		else //down
		{
			a = {overlap_area.max.x, overlap_area.min.y};
			c = {min_2.x, min_2.y};
			d = {min_2.x, max_2.y};
			e = {max_2.x, max_2.y};
			normal_1 = left;
			normal_2 = up;
		}
	}
	else //left
	{
		if(up_dot > 0.0f) //up
		{
			a = {overlap_area.min.x, overlap_area.max.y};
			c = {max_2.x, max_2.y};
			d = {max_2.x, min_2.y};
			e = {min_2.x, min_2.y};
			normal_1 = right;
			normal_2 = down;
		}
		else //down
		{
			a = overlap_area.min;
			c = {max_2.x, min_2.y};
			d = {max_2.x, max_2.y};
			e = {min_2.x, max_2.y};
			normal_1 = right;
			normal_2 = up;
		}
	}
	b = a - sweep_vector;

	math::line_intersection intersection = line_vs_line_intersection(a, b, c, d);
	if(intersection.found)
	{
		result.position = intersection.position;
		result.normal = normal_1;
	}
	else
	{
		intersection = line_vs_line_intersection(a, b, d, e);
		assert(intersection.found);
		result.position = intersection.position;
		result.normal = normal_2;
	}

	result.distance_on_sweep_vector = length(result.position - b);

	return result;
}

internal inline
shape aabb_shape(vector2 center, vector2 half_extents)
{
	shape result;
	result.type = aabb;
	result.points[0] = center - half_extents;
	result.points[1] = {center.x+half_extents.x, result.points[0].y};
	result.points[2] = center + half_extents;
	result.points[3] = {result.points[0].x, result.points[2].y};
	return result;
}

#define MAX_SHAPES 16
struct aabb_linear_sweep_result
{
	sweep_result sweep_results[MAX_SHAPES];
	uint32 num_sweep_results;
};

internal inline
aabb_linear_sweep_result aabb_linear_sweep(vector2 from, vector2 to, vector2 aabb_half_extents, shape* shapes, uint32 num_shapes)
{
	aabb_linear_sweep_result result;
	result.num_sweep_results = 0;

	if(num_shapes == 0)
	{
		return result;
	}

	shape sweep_shape = aabb_shape(from, aabb_half_extents);

	//todo(staffan): maybe worth making this more accurate?
	float32 half_extents_length = math::min(aabb_half_extents.x, aabb_half_extents.y);

	vector2 movement = to-from;
	float32 movement_length = length(movement);
	vector2 movement_dir = normalize(movement);

	uint32 num_overlapped_shapes = 0;

	bool already_collided[MAX_SHAPES] = {};
	assert(num_shapes <= MAX_SHAPES);
	for(uint32 i = 0; i<num_shapes; ++i)
	{
		shape* current_shape = shapes+i;
		switch(current_shape->type)
		{
			case aabb:
			{
				if(aabb_vs_aabb_overlap(sweep_shape, *current_shape))
				{
					already_collided[i] = true;
					++num_overlapped_shapes;
				}

			} break;
			default: {assert(false);} break;
		}
	}

	uint32 iterations = (uint32)math::ceil(movement_length/half_extents_length);
	float32 step_size = movement_length/iterations;
	math::vector2 step_vector = movement_dir*step_size;

	uint32 num_results = 0;

	for(uint32 step_i=0; step_i<iterations; ++step_i)
	{
		for(uint32 point_i=0; point_i<4; ++point_i)
		{
			sweep_shape.points[point_i] += step_vector;
		}

		for(uint32 shape_i=0; shape_i<num_shapes; ++shape_i)
		{
			if(already_collided[shape_i])
			{
				continue;
			}

			shape* current = shapes+shape_i;

			sweep_result sr;
			switch(current->type)
			{
				case aabb:
				{
					sr = aabb_vs_aabb_sweep(sweep_shape, *current, step_vector);
				} break;
				default:
				{
					sr = {};
					assert(false);
				} break;
			}

			if(sr.collision)
			{
				sr.distance_on_sweep_vector += step_size*step_i;

				if(result.num_sweep_results == 0)
				{
					result.sweep_results[result.num_sweep_results++] = sr;
				}
				else
				{
					uint32 index = result.num_sweep_results;
					for(int32 sr_i = (result.num_sweep_results-1); sr_i>=0; --sr_i)
					{
						if(sr.distance_on_sweep_vector < result.sweep_results[sr_i].distance_on_sweep_vector)
						{
							index = sr_i;
						}
						else
						{
							break;
						}
					}

					if(index < result.num_sweep_results)
					{
						for(int32 sr_i = (result.num_sweep_results-1); sr_i>=(int32)index; --sr_i)
						{
							result.sweep_results[sr_i+1] = result.sweep_results[sr_i];
						}
					}
					result.sweep_results[index] = sr;
					++result.num_sweep_results;
				}

				if(++num_overlapped_shapes == num_shapes)
				{
					goto break_sweep;
				}
				already_collided[shape_i] = true;
			}
		}
	}
	break_sweep:

	return result;
}

}//namespace math