namespace math
{
global_variable constexpr f32 deg_to_rad = PI/180.0f;
global_variable constexpr f32 rad_to_deg = 180.0f/PI;
global_variable constexpr f32 huge = (3.4e+38f);

#include <math.h> //todo(staffan): get rid of this.
internal inline i32 ceil_float_to_int(f32 value)
{
	i32 result = (i32)ceilf(value);
	return result;
}

internal inline i32 round_f32_to_i32(f32 value)
{
	i32 result = (i32)roundf(value);
	return result;
}

internal inline u32 round_f32_to_u32(f32 value)
{
	u32 result = (u32)(value+0.5f);
	return result;
}

internal inline i32 floor_float_to_int(f32 value)
{
	i32 result = (i32)floorf(value);
	return result;
}

internal inline f32 lerp(f32 value1, f32 value2, f32 t)
{
	f32 result = ((1.0f-t)*value1) + (t*value2);
	return result;
}

internal inline
f32 square_root(f32 value)
{
	f32 result;
	result = (f32)sqrt(value);
	return result;
}

internal inline
i32 sign(f32 value)
{
	i32 result;
	result = signbit(value) ? -1 : 1;
	return result;
}

internal inline
f32 cos(f32 v)
{
	f32 result = cosf(v);
	return result;
}

internal inline
f32 sin(f32 v)
{
	f32 result = sinf(v);
	return result;
}

internal inline
f32 min(f32 a, f32 b)
{
	f32 result = (a<b) ? a : b;
	return result;
}

internal inline
f32 max(f32 a, f32 b)
{
	f32 result = (a>b) ? a : b;
	return result;
}

internal inline
u32 min(u32 a, u32 b)
{
	u32 result = (a<b) ? a : b;
	return result;
}

internal inline
u32 max(u32 a, u32 b)
{
	u32 result = (a>b) ? a : b;
	return result;
}

internal inline
u16 min(u16 a, u16 b)
{
	u16 result = (a<b) ? a : b;
	return result;
}

internal inline
u16 max(u16 a, u16 b)
{
	u16 result = (a>b) ? a : b;
	return result;
}

internal inline
f32 abs(f32 a)
{
	f32 result = (f32)fabs(a);
	return result;
}

internal inline
f32 tangent(f32 a)
{
	f32 result = tanf(a);
	return result;
}

}//namespace math

#include "vector2.h"
#include "vector3.h"
#include "matrix.h"
#include "quaternion.h"

namespace math
{

struct line_intersection
{
	bool found;
	bool collinear;
	v2 position;
	v2 position2;
};

internal inline
line_intersection line_vs_line_intersection(v2 a, v2 b, v2 c, v2 d)
{
	//https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect#
	line_intersection result;

	v2 p = a;
	v2 r = b-a;
	v2 q = c;
	v2 s = d-c;

	v2 pq = q-p;

	f32 r_s_cross = cross(r, s);
	f32 pq_s_cross = cross(pq, s);
	f32 pq_r_cross = cross(pq, r);

	f32 t = pq_s_cross / r_s_cross;
	f32 u = pq_r_cross / r_s_cross;

	if(r_s_cross == 0.0f && pq_r_cross == 0.0f)
	{
		//lines are collinear, check for overlap
		f32 rr_dot = dot(r, r);
		f32 sr_dot = dot(s, r);

		f32 t0 = dot(pq, r) / rr_dot;
		f32 t1 = t0 + sr_dot / rr_dot;

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
bool aabb_vs_aabb_overlap(v2 a_min, v2 a_max, v2 b_min, v2 b_max)
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
	v2 min;
	v2 max;
};
internal inline
aabb_overlap_area_result aabb_vs_aabb_overlap_area(v2 a_min, v2 a_max, v2 b_min, v2 b_max)
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
	v2 points[4];
};

internal inline
bool aabb_vs_aabb_overlap(shape aabb_1, shape aabb_2)
{
	bool result;
	v2 min_1, max_1, min_2, max_2;
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
	v2 position;
	v2 normal;
	f32 distance_on_sweep_vector;
};
internal inline
sweep_result aabb_vs_aabb_sweep(shape aabb_1, shape aabb_2, v2 sweep_vector)
{
	sweep_result result;

	v2 min_1, max_1, min_2, max_2;
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

	const_exp math::v2 right = {1.0f, 0.0f};
	const_exp math::v2 up = {0.0f, 1.0f};
	const_exp math::v2 left = {-1.0f, 0.0f};
	const_exp math::v2 down = {0.0f, -1.0f};
	
	math::v2 a, b, c, d, e, normal_1, normal_2;

	f32 right_dot = math::dot(sweep_vector, right);
	f32 up_dot = math::dot(sweep_vector, up);

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
shape aabb_shape(v2 center, v2 half_extents)
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
	u32 num_sweep_results;
};

internal inline
aabb_linear_sweep_result aabb_linear_sweep(v2 from, v2 to, v2 aabb_half_extents, shape* shapes, u32 num_shapes)
{
	aabb_linear_sweep_result result;
	result.num_sweep_results = 0;

	if(num_shapes == 0)
	{
		return result;
	}

	shape sweep_shape = aabb_shape(from, aabb_half_extents);

	//todo(staffan): maybe worth making this more accurate?
	f32 half_extents_length = math::min(aabb_half_extents.x, aabb_half_extents.y);

	v2 movement = to-from;
	f32 movement_length = length(movement);
	v2 movement_dir = normalize(movement);

	u32 num_overlapped_shapes = 0;

	bool already_collided[MAX_SHAPES] = {};
	assert(num_shapes <= MAX_SHAPES);
	for(u32 i = 0; i<num_shapes; ++i)
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

	u32 iterations = (u32)math::ceil(movement_length/half_extents_length);
	f32 step_size = movement_length/iterations;
	math::v2 step_vector = movement_dir*step_size;

	u32 num_results = 0;

	for(u32 step_i=0; step_i<iterations; ++step_i)
	{
		for(u32 point_i=0; point_i<4; ++point_i)
		{
			sweep_shape.points[point_i] += step_vector;
		}

		for(u32 shape_i=0; shape_i<num_shapes; ++shape_i)
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
					u32 index = result.num_sweep_results;
					for(i32 sr_i = (result.num_sweep_results-1); sr_i>=0; --sr_i)
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
						for(i32 sr_i = (result.num_sweep_results-1); sr_i>=(i32)index; --sr_i)
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