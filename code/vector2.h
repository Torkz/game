
namespace math
{

union v2
{
	struct
	{
		f32 x, y;
	};
	f32 elements[2];
};

inline v2
operator-(v2 v)\
{
	v2 result;
	result.x = -v.x;
	result.y = -v.y;
	return result;
};

inline v2
operator-(v2 a, v2 b)\
{
	v2 result;
	result.x = a.x-b.x;
	result.y = a.y-b.y;
	return result;
};

inline void
operator-=(v2& a, v2 b)
{
	a = a - b;
};

inline v2
operator+(v2 a, v2 b)\
{
	v2 result;
	result.x = a.x+b.x;
	result.y = a.y+b.y;
	return result;
};

inline void
operator+=(v2& a, v2 b)\
{
	a = a + b;
};

inline v2
operator*(v2 a, v2 b)\
{
	v2 result;
	result.x = a.x*b.x;
	result.y = a.y*b.y;
	return result;
};

inline void
operator*=(v2& a, v2 b)
{
	a = a*b;
};

inline v2
operator*(v2 v, f32 scale)\
{
	v2 result;
	result.x = v.x*scale;
	result.y = v.y*scale;
	return result;
};

inline v2
operator*(f32 scale, v2 v)\
{
	v2 result;
	result = v*scale;
	return result;
};

inline void
operator*=(v2& v, f32 scale)\
{
	v = v * scale;
};

inline v2
operator/(v2 v, f32 scale)\
{
	v2 result;
	result.x = v.x / scale;
	result.y = v.y / scale;
	return result;
}

inline f32
length_squared(v2 v)
{
	f32 result;
	result = v.x*v.x + v.y*v.y;
	return result;
}

inline f32
length(v2 v)
{
	f32 result;
	result = math::square_root(length_squared(v));
	return result;
}

inline v2
normalize(v2 v)
{
	v2 result;
	result = v / length(v);
	return result;
}

inline f32
dot(v2 a, v2 b)
{
	f32 result;
	result = a.x*b.x + a.y*b.y;
	return result;
}

inline f32
cross(v2 a, v2 b)
{
	f32 result;
	result = a.x*b.y - a.y*b.x;
	return result;
}

inline v2
lerp(v2 a, v2 b, f32 t)
{
	v2 result;
	result = a + (b-a)*t;
	return result;
}

}//namespace math