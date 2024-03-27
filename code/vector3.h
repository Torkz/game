union v3
{
    struct
    {
        f32 x, y, z;
    };
    f32 elements[3];
};

inline v3
operator-(v3 a, v3 b)
{
    v3 result;
    result.x = a.x-b.x;
    result.y = a.y-b.y;
    result.z = a.z-b.z;
    return result;
}

inline v3
operator/(v3 v, f32 scale)
{
    v3 result;
    result.x = v.x / scale;
    result.y = v.y / scale;
    result.z = v.z / scale;
    return result;
}

inline v3
operator+(v3 a, v3 b)
{
    v3 result;
    result.x = a.x+b.x;
    result.y = a.y+b.y;
    result.z = a.z+b.z;
    return result;
}

inline void
operator+=(v3& a, v3 b)
{
    a = a+b;
}

inline v3
operator*(v3 a, f32 scale)
{
    v3 result;
    result.x = a.x*scale;
    result.y = a.y*scale;
    result.z = a.z*scale;
    return result;
}

inline void
operator*=(v3& a, f32 scale)
{
    a = a*scale;
}

inline v3
operator*(f32 scale, v3 a)
{
    v3 result;
    result.x = a.x*scale;
    result.y = a.y*scale;
    result.z = a.z*scale;
    return result;
}

inline void
operator*=(f32 scale, v3& a)
{
    a = a*scale;
}

inline f32
length_squared(v3 v)
{
	f32 result;
	result = v.x*v.x + v.y*v.y + v.z*v.z;
	return result;
}

inline f32
length(v3 v)
{
	f32 result;
	result = math::square_root(length_squared(v));
	return result;
}

inline v3
normalize(v3 v)
{
	v3 result;
	result = v / length(v);
	return result;
}

inline f32
dot(v3 a, v3 b)
{
	f32 result;
	result = a.x*b.x + a.y*b.y + a.z*b.z;
	return result;
}

internal inline
v3 cross(v3 a, v3 b)
{
    v3 result;
    result.x = a.y*b.z - a.z*b.y;
    result.y = a.z*b.x - a.x*b.z;
    result.z = a.x*b.y - a.y*b.x;
    return result;
}