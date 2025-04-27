struct v4
{
	f32 x, y, z, w;
};

internal inline
v4 vector4(const v3& v, f32 w)
{
	v4 result = {v.x, v.y, v.z, w};
	return result;
}