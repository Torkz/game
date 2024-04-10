struct m4x4
{
	internal constexpr u8 rows = 4;
	internal constexpr u8 columns = 4;
	f32 elements[rows][columns];

	inline f32* operator[](i32 i)
	{
		assert(i>=0 && i<rows);
		return elements[i];
	}

	internal inline constexpr
	m4x4 identity()
	{
		m4x4 result = {};
		for(u8 row = 0; row<rows; ++row)
		{
			for(u8 column = 0; column<columns; ++column)
			{
				result[row][column] = (row==column) ? 1.0f : 0.0f;
			}
		}
		return result;
	}

	internal inline
	v3 multiply_vector(m4x4& m, v3& p)
	{
		v3 result = {};
		result.x = (p.x*m[0][0] + p.y*m[1][0] + p.z*m[2][0] + m[3][0]);
		result.y = (p.x*m[0][1] + p.y*m[1][1] + p.z*m[2][1] + m[3][1]);
		result.z = (p.x*m[0][2] + p.y*m[1][2] + p.z*m[2][2] + m[3][2]);
		f32 w = p.x*m[0][3] + p.y*m[1][3] + p.z*m[2][3] + m[3][3];
		if(w != 0.0f)
		{
			result.x /= w;
			result.y /= w;
			result.z /= w;
		}
		return result;
	};

	internal inline
	m4x4 make_rotation_x(f32 angle_rad)
	{
		m4x4 result = {0.0f};
		result[0][0] = 1.0f;
		result[1][1] = math::cos(angle_rad);
		result[1][2] = math::sin(angle_rad);
		result[2][1] = -math::sin(angle_rad);
		result[2][2] = math::cos(angle_rad);
		result[3][3] = 1.0f;
		return result;
	}

	internal inline
	m4x4 make_rotation_y(f32 angle_rad)
	{
		m4x4 result = {0.0f};
		result[0][0] = math::cos(angle_rad);
		result[0][2] = math::sin(angle_rad);
		result[2][0] = -math::sin(angle_rad);
		result[1][1] = 1.0f;
		result[2][2] = math::cos(angle_rad);
		result[3][3] = 1.0f;
		return result;
	}

	internal inline
	m4x4 make_rotation_z(f32 angle_rad)
	{
		m4x4 result = {0.0f};
		result[0][0] = math::cos(angle_rad);
		result[0][1] = math::sin(angle_rad);
		result[1][0] = -math::sin(angle_rad);
		result[1][1] = math::cos(angle_rad);
		result[2][2] = 1.0f;
		result[3][3] = 1.0f;
		return result;
	}

	internal inline
	m4x4 make_translation(v3& translation)
	{
		m4x4 result = {0.0f};
		result[0][0] = 1.0f;
		result[1][1] = 1.0f;
		result[2][2] = 1.0f;
		result[3][3] = 1.0f;
		result[3][0] = translation.x;
		result[3][1] = translation.y;
		result[3][2] = translation.z;
		return result;
	}

	internal inline
	m4x4 look_at(v3& position, v3& look_at, v3& up)
	{
		assert(!(length_squared(look_at) > 1.0f));
		v3 up_correction = look_at*dot(up, look_at);
		v3 real_up = normalize(up-up_correction);

		v3 right = cross(real_up, look_at);
		m4x4 result;
		result[0][0] = right.x;
		result[0][1] = right.y;
		result[0][2] = right.z;
		result[0][3] = 0.0f;

		result[1][0] = look_at.x;
		result[1][1] = look_at.y;
		result[1][2] = look_at.z;
		result[1][3] = 0.0f;

		result[2][0] = up.x;
		result[2][1] = up.y;
		result[2][2] = up.z;
		result[2][3] = 0.0f;

		result[3][0] = position.x;
		result[3][1] = position.y;
		result[3][2] = position.z;
		result[3][3] = 1.0f;
		return result;
	}

	internal inline
	m4x4 quick_inverse(m4x4& m) //only for rotation/translation matrices.
	{
		m4x4 result;
		result[0][0] = m[0][0];
		result[0][1] = m[1][0];
		result[0][2] = m[2][0];
		result[0][3] = 0.0f;

		result[1][0] = m[0][1];
		result[1][1] = m[1][1];
		result[1][2] = m[2][1];
		result[1][3] = 0.0f;
		
		result[2][0] = m[0][2];
		result[2][1] = m[1][2];
		result[2][2] = m[2][2];
		result[2][3] = 0.0f;

		result[3][0] = -(m[3][0]*result[0][0] +
						 m[3][1]*result[1][0] +
						 m[3][2]*result[2][0]);
		result[3][1] = -(m[3][0]*result[0][1] +
						 m[3][1]*result[1][1] +
						 m[3][2]*result[2][1]);
		result[3][2] = -(m[3][0]*result[0][2] +
						 m[3][1]*result[1][2] +
						 m[3][2]*result[2][2]);
		result[3][3] = 1.0f;
		return result;
	}
};

inline
m4x4 operator*(m4x4 a, m4x4 b)
{
	m4x4 result = {};
	result[0][0] = a[0][0]*b[0][0] + a[0][1]*b[1][0] + a[0][2]*b[2][0] + a[0][3]*b[3][0];
	result[0][1] = a[0][0]*b[0][1] + a[0][1]*b[1][1] + a[0][2]*b[2][1] + a[0][3]*b[3][1];
	result[0][2] = a[0][0]*b[0][2] + a[0][1]*b[1][2] + a[0][2]*b[2][2] + a[0][3]*b[3][2];
	result[0][3] = a[0][0]*b[0][3] + a[0][1]*b[1][3] + a[0][2]*b[2][3] + a[0][3]*b[3][3];

	result[1][0] = a[1][0]*b[0][0] + a[1][1]*b[1][0] + a[1][2]*b[2][0] + a[1][3]*b[3][0];
	result[1][1] = a[1][0]*b[0][1] + a[1][1]*b[1][1] + a[1][2]*b[2][1] + a[1][3]*b[3][1];
	result[1][2] = a[1][0]*b[0][2] + a[1][1]*b[1][2] + a[1][2]*b[2][2] + a[1][3]*b[3][2];
	result[1][3] = a[1][0]*b[0][3] + a[1][1]*b[1][3] + a[1][2]*b[2][3] + a[1][3]*b[3][3];

	result[2][0] = a[2][0]*b[0][0] + a[2][1]*b[1][0] + a[2][2]*b[2][0] + a[2][3]*b[3][0];
	result[2][1] = a[2][0]*b[0][1] + a[2][1]*b[1][1] + a[2][2]*b[2][1] + a[2][3]*b[3][1];
	result[2][2] = a[2][0]*b[0][2] + a[2][1]*b[1][2] + a[2][2]*b[2][2] + a[2][3]*b[3][2];
	result[2][3] = a[2][0]*b[0][3] + a[2][1]*b[1][3] + a[2][2]*b[2][3] + a[2][3]*b[3][3];

	result[3][0] = a[3][0]*b[0][0] + a[3][1]*b[1][0] + a[3][2]*b[2][0] + a[3][3]*b[3][0];
	result[3][1] = a[3][0]*b[0][1] + a[3][1]*b[1][1] + a[3][2]*b[2][1] + a[3][3]*b[3][1];
	result[3][2] = a[3][0]*b[0][2] + a[3][1]*b[1][2] + a[3][2]*b[2][2] + a[3][3]*b[3][2];
	result[3][3] = a[3][0]*b[0][3] + a[3][1]*b[1][3] + a[3][2]*b[2][3] + a[3][3]*b[3][3];
	return result;
}

inline void
operator*=(m4x4& a, m4x4& b)
{
	a = a*b;
}