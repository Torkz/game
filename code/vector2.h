
namespace math
{

union vector2
{
	struct
	{
		float32 x, y;
	};
	float32 element[2];
};

inline vector2
operator-(vector2 v)\
{
	vector2 result;
	result.x = -v.x;
	result.y = -v.y;
	return result;
};

inline vector2
operator-(vector2 v1, vector2 v2)\
{
	vector2 result;
	result.x = v1.x-v2.x;
	result.y = v1.y-v2.y;
	return result;
};

inline void
operator-=(vector2& v1, vector2 v2)
{
	v1 = v1 - v2;
};

inline vector2
operator+(vector2 v1, vector2 v2)\
{
	vector2 result;
	result.x = v1.x+v2.x;
	result.y = v1.y+v2.y;
	return result;
};

inline void
operator+=(vector2& v1, vector2 v2)\
{
	v1 = v1 + v2;
};

inline vector2
operator*(vector2 v1, vector2 v2)\
{
	vector2 result;
	result.x = v1.x*v2.x;
	result.y = v1.y*v2.y;
	return result;
};

inline void
operator*=(vector2& v1, vector2 v2)
{
	v1 = v1*v2;
};

inline vector2
operator*(vector2 v, float32 scale)\
{
	vector2 result;
	result.x = v.x*scale;
	result.y = v.y*scale;
	return result;
};

inline vector2
operator*(float32 scale, vector2 v)\
{
	vector2 result;
	result = v*scale;
	return result;
};

inline void
operator*=(vector2& v, float32 scale)\
{
	v = v * scale;
};

inline vector2
operator/(vector2 v, float32 scale)\
{
	vector2 result;
	result.x = v.x / scale;
	result.y = v.y / scale;
	return result;
}

inline float32
length_squared(vector2 v)
{
	float32 result;
	result = v.x*v.x + v.y*v.y;
	return result;
}

inline float32
length(vector2 v)
{
	float32 result;
	result = math::square_root(length_squared(v));
	return result;
}

inline vector2
normalize(vector2 v)
{
	vector2 result;
	result = v / length(v);
	return result;
}

inline float32
dot(vector2 v1, vector2 v2)
{
	float32 result;
	result = v1.x*v2.x + v1.y*v2.y;
	return result;
}

inline float32
cross(vector2 v1, vector2 v2)
{
	float32 result;
	result = v1.x*v2.y - v1.y*v2.x;
	return result;
}

inline vector2
lerp(vector2 v1, vector2 v2, float32 t)
{
	vector2 result;
	result = v1 + (v2-v1)*t;
	return result;
}

}//namespace math