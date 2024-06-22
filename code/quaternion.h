union quaternion
{
	struct
	{
		f32 w, x, y, z;
	};
	struct
	{
		f32 s;
		v3 v;
	};	
	f32 elements[4];
};

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
	v3 v_result = cross(a.v, b.v) + a.s*b.v + b.s*a.v;

	quaternion result;
	result.w = a.s*b.s - dot(a.v, b.v);
	result.x = v_result.x;
	result.y = v_result.y;
	result.z = v_result.z;
	return result;
}

inline quaternion
operator/(const quaternion& a, const quaternion&b)
{
	v3 v_result = cross(-a.v, b.v) - a.s*b.v + b.s*a.v;

	quaternion result;
	result.w = a.s*b.s + dot(a.v, b.v);
	result.x = v_result.x;
	result.y = v_result.y;
	result.z = v_result.z;
	return result;
}

inline quaternion
conjugate(const quaternion& q)
{
	quaternion result;
	result.s = q.s;
	result.v = -q.v;
	return result;
}

inline f32
length(const quaternion& q)
{
	f32 result = (f32)math::sqrt(q.x*q.x + q.y*q.y + q.z*q.z + q.w*q.w);
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