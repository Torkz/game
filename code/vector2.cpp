
namespace game
{

struct vector2
{
	union
	{
		float32 x, y;
		float32 element[2];
	};
};

vector2 operator-(vector2 v)\
{
	vector2 result;
	result.x = -v.x;
	result.y = -v.y;
	return result;
};

vector2 operator+(vector2 v1, vector2 v2)\
{
	vector2 result;
	result.x = v1.x+v2.x;
	result.y = v1.y+v2.y;
	return result;
};

vector2 operator-(vector2 v1, vector2 v2)\
{
	vector2 result;
	result.x = v1.x-v2.x;
	result.y = v1.y-v2.y;
	return result;
};

vector2 operator*(vector2 v, float32 scale)\
{
	vector2 result;
	result.x = v.x*scale;
	result.y = v.y*scale;
	return result;
};

}//namespace game