struct quaternion {
	f32 w, x, y, z;
};

static constexpr quaternion
identity()
{
	return {1.0f, 0.0f, 0.0f, 0.0f};
}

//https://www.euclideanspace.com/maths/algebra/realNormedAlgebra/quaternions/arithmetic/index.htm
inline quaternion
operator+(const quaternion &a, const quaternion &b)
{
	quaternion result;
	result.w = a.w+b.w;
	result.x = a.x+b.x;
	result.y = a.y+b.y;
	result.z = a.z+b.z;
	return result;
}

inline void
operator+=(quaternion &a, const quaternion &b)
{
	a = a+b;
}

inline quaternion
operator-(const quaternion &a, const quaternion &b)
{
	quaternion result;
	result.w = a.w-b.w;
	result.x = a.x-b.x;
	result.y = a.y-b.y;
	result.z = a.z-b.z;
	return result;
}

inline quaternion
operator*(const quaternion &a, const quaternion &b)
{
	const v3* a_v = (v3*)&a.x;
	const v3* b_v = (v3*)&b.x;

	v3 v_result = cross(*a_v, *b_v) + a.w*(*b_v) + b.w*(*a_v);

	quaternion result;
	result.w = a.w*b.w - dot(*a_v, *b_v);
	result.x = v_result.x;
	result.y = v_result.y;
	result.z = v_result.z;
	return result;
}

inline quaternion
operator/(const quaternion& a, const quaternion& b)
{
	const v3* a_v = (v3*)&a.x;
	const v3* b_v = (v3*)&b.x;

	v3 v_result = cross(-(*a_v), *b_v) - a.w*(*b_v) + b.w*(*a_v);

	quaternion result;
	result.w = a.w*b.w + dot(*a_v, *b_v);
	result.x = v_result.x;
	result.y = v_result.y;
	result.z = v_result.z;
	return result;
}

inline quaternion
conjugate(const quaternion& q)
{
	const v3* q_v = (v3*)&q.x;

	quaternion result;
	result.w = q.w;
	v3* r_v = (v3*)&result.x;
	*r_v = -(*q_v);
	return result;
}

inline f32
length(const quaternion& q)
{
	f32 result = (f32)math::sqrt(q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z);
	return result;
}

inline f32
length_squared(const quaternion& q)
{
	f32 result = q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z;
	return result;
}

inline quaternion
normalize(const quaternion& q)
{
	f32 q_length = length(q);
	quaternion result;
	result.w = q.w/q_length;
	result.x = q.x/q_length;
	result.y = q.y/q_length;
	result.z = q.z/q_length;
	return result;
}

inline f32
dot(const quaternion& lhs, const quaternion& rhs)
{
	f32 d = lhs.w*rhs.w + lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z;
	return d;
}

internal constexpr f32 quaternion_rotation_tolerance = 1e-6f;

//https://www.sjbrown.co.uk/posts/representing-rotations-in-quaternion-arithmetic/
inline internal v3
right(const quaternion& q)
{
	assert(math::abs(1.0f - length_squared(q)) < quaternion_rotation_tolerance);
	const f32& w = q.w;
	const f32& x = q.x;
	const f32& y = q.y;
	const f32& z = q.z;

	const f32 y2 = y*y;
	const f32 z2 = z*z;

	v3 result = {1.0f - 2.0f*y2 - 2.0f*z2, 2.0f*x*y + 2.0f*w*z, 2.0f*z*x - 2.0f*w*y};
	return result;
}

inline internal v3
forward(const quaternion& q)
{
	assert(math::abs(1.0f - length_squared(q)) < quaternion_rotation_tolerance);
	const f32& w = q.w;
	const f32& x = q.x;
	const f32& y = q.y;
	const f32& z = q.z;

	const f32 x2 = x*x;
	const f32 z2 = z*z;

	v3 result = {2.0f*x*y - 2.0f*w*z, 1.0f - 2.0f*x2 - 2.0f*z2, 2.0f*y*z + 2.0f*w*x};
	return result;
}

inline internal v3
up(const quaternion& q)
{
	assert(math::abs(1.0f - length_squared(q)) < quaternion_rotation_tolerance);
	const f32& w = q.w;
	const f32& x = q.x;
	const f32& y = q.y;
	const f32& z = q.z;

	const f32 x2 = x*x;
	const f32 y2 = y*y;

	v3 result = {2.0f*z*x + 2.0f*w*y, 2.0f*y*z - 2.0f*w*x, 1.0f - 2.0f*x2 - 2.0f*y2};
	return result;
}

inline internal m4x4
matrix(const quaternion& q)
{
	f32 d = dot(q, q);
	assert(math::abs(1.0f - length_squared(q)) < quaternion_rotation_tolerance);
	const f32& w = q.w;
	const f32& x = q.x;
	const f32& y = q.y;
	const f32& z = q.z;

	const f32 x2 = x*x;
	const f32 y2 = y*y;
	const f32 z2 = z*z;

	m4x4 result = m4x4::identity();
	x_axis(result) = {1.0f - 2.0f*y2 - 2.0f*z2, 2.0f*x*y + 2.0f*w*z, 2.0f*z*x - 2.0f*w*y};
	y_axis(result) = {2.0f*x*y - 2.0f*w*z, 1.0f - 2.0f*x2 - 2.0f*z2, 2.0f*y*z + 2.0f*w*x};
	z_axis(result) = {2.0f*z*x + 2.0f*w*y, 2.0f*y*z - 2.0f*w*x, 1.0f - 2.0f*x2 - 2.0f*y2};
	return result;
}

inline internal v3
rotate(const v3& a, const quaternion& b)
{
	quaternion v_q = {0.0f, a.x, a.y, a.z};
	quaternion result = b*v_q*conjugate(b);
	return *(v3*)&result.x;
}

inline internal quaternion
from_angle_axis(f32 angle_rad, const v3& axis)
{
	f32 half_angle_rad = angle_rad*0.5f;
	f32 cos = math::cos(half_angle_rad);
	f32 sin = math::sin(half_angle_rad);
	quaternion result = {
		cos,
		axis.x*sin,
		axis.y*sin,
		axis.z*sin
	};
	return result;
}