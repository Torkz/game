struct m4x4
{
	const_exp constexpr u8 rows = 4;
	const_exp constexpr u8 columns = 4;
	f32 elements[rows*columns];

	internal inline constexpr
	m4x4 identity()
	{
		m4x4 result = {};
		for(u8 row = 0; row<rows; ++row)
		{
			for(u8 column = 0; column<columns; ++column)
			{
				result.elements[row*rows + column] = (row==column) ? 1.0f : 0.0f;
			}
		}
		return result;
	}

	// internal inline
	// m4x4 from_euler(f32 yaw, f32 pitch, f32 roll)
	// {
	// 	const f32 cr = math::cos(roll);
	// 	const f32 cy = math::cos(yaw);
	// 	const f32 sy = math::sin(yaw);

	// 	m4x4 result = identity();
	// 	x_axis(result) = {cr*cy, -(cr*sy), math::sin(roll)};
	// 	y_axis(result) = {math::cos(pitch)}
	// 	return result;
	// }

	internal inline
	v3 multiply_vector(const m4x4& m, const v3& p)
	{
		v3 result = {};

		const f32* m_p = &m.elements[0];
		f32* p_p = (f32*)&p;
		f32* dst_p = (f32*)&result;
		for(i8 i=0; i<3; ++i)
		{
			*dst_p++ =
				p_p[0]*m_p[0+i] +
				p_p[1]*m_p[4+i] +
				p_p[2]*m_p[8+i] +
				m_p[12+i];
		}
		f32 w = p.x*m_p[0] + p.y*m_p[4] + p.z*m_p[8] + m_p[12];
		if(w != 0.0f)
		{
			result.x /= w;
			result.y /= w;
			result.z /= w;
		}
		return result;
	};

	internal inline
	v4 multiply(const m4x4& m, const v4& v)
	{
		v4 result = {};
		const f32* m_p = &m.elements[0];
		f32* v_p = (f32*)&v;
		f32* dst_p = (f32*)&result;
		for(i8 i=0; i<4; ++i)
		{
			*dst_p++ = 
				v_p[0]*m_p[0+i] +
				v_p[1]*m_p[4+i] +
				v_p[2]*m_p[8+i] +
				v_p[3]*m_p[12+i];
		}
		return result;
	}

	internal inline
	v3 multiply(const m4x4& m, const v3& v)
	{
		v3 result = {};
		const f32* m_p = &m.elements[0];
		f32* v_p = (f32*)&v;
		f32* dst_p = (f32*)&result;
		for(i8 i=0; i<3; ++i)
		{
			*dst_p++ = 
				v_p[0]*m_p[0+i] +
				v_p[1]*m_p[4+i] +
				v_p[2]*m_p[8+i] +
				m_p[12+i];
		}
		return result;
	}

	internal inline
	m4x4 make_rotation_x(f32 angle_rad)
	{
		f32 cos = math::cos(angle_rad);
		f32 sin = math::sin(angle_rad);

		m4x4 result = m4x4::identity();
		f32* m = &result.elements[0];
		m[5] = cos;
		m[6] = -sin;
		m[9] = sin;
		m[10] = cos;
		return result;
	}

	internal inline
	m4x4 make_rotation_y(f32 angle_rad)
	{
		f32 cos = math::cos(angle_rad);
		f32 sin = math::sin(angle_rad);

		m4x4 result = m4x4::identity();
		f32* m = &result.elements[0];
		m[0] = cos;
		m[2] = sin;
		m[8] = -sin;
		m[10] = cos;
		return result;
	}

	internal inline
	m4x4 make_rotation_z(f32 angle_rad)
	{
		f32 cos = math::cos(angle_rad);
		f32 sin = math::sin(angle_rad);

		m4x4 result = m4x4::identity();
		f32* m = &result.elements[0];
		m[0] = cos;
		m[1] = -sin;
		m[4] = sin;
		m[5] = cos;
		return result;
	}

	internal inline
	m4x4 make_translation(const v3& translation)
	{
		m4x4 result = m4x4::identity();
		f32* m = &result.elements[0];
		m[12] = translation.x;
		m[13] = translation.y;
		m[14] = translation.z;
		return result;
	}

	internal inline
	m4x4 look_at(const v3& position, const v3& look_at_position, const v3& up)
	{
		const v3 look_at = normalize(look_at_position-position);
		f32 d = dot(up, look_at);
		assert(d<1.0f);
		const v3 up_correction = look_at*d;
		v3 real_up = normalize(up-up_correction);

		const v3 right = cross(look_at, real_up);
		m4x4 result = identity();
		f32* m = &result.elements[0];
		m[0] = right.x;
		m[1] = right.y;
		m[2] = right.z;

		m[4] = look_at.x;
		m[5] = look_at.y;
		m[6] = look_at.z;

		m[8] = up.x;
		m[9] = up.y;
		m[10] = up.z;

		m[12] = position.x;
		m[13] = position.y;
		m[14] = position.z;
		return result;
	}

	internal inline
	m4x4 quick_inverse(const m4x4& m) //only for rotation/translation matrices.
	{
		m4x4 result = identity();
		f32* r_p = &result.elements[0];
		const f32* m_p = &m.elements[0];
		r_p[0] = m_p[0];
		r_p[1] = m_p[4];
		r_p[2] = m_p[8];

		r_p[4] = m_p[1];
		r_p[5] = m_p[5];
		r_p[6] = m_p[9];
		
		r_p[8] = m_p[2];
		r_p[9] = m_p[6];
		r_p[10] = m_p[10];

		r_p[12] = -(m_p[12]*r_p[0] +
					m_p[13]*r_p[4] +
					m_p[14]*r_p[8]);
		r_p[13] = -(m_p[12]*r_p[1] +
					m_p[13]*r_p[5] +
					m_p[14]*r_p[9]);
		r_p[14] = -(m_p[12]*r_p[2] +
					m_p[13]*r_p[6] +
					m_p[14]*r_p[10]);
		return result;
	}

	internal inline
	m4x4 perspective(f32 fov_radians, f32 aspect_ratio, f32 near_clip, f32 far_clip)
	{
		f32 half_tan_fov = math::tangent(fov_radians);
		m4x4 result = {0.0f};
		f32* m = &result.elements[0];
		m[0] = 1.0f / (aspect_ratio * half_tan_fov);
		m[5] = -((far_clip + near_clip) / (far_clip - near_clip));
		m[7] = -1.0f;
		m[10] = 1.0f / half_tan_fov;
		m[13] = -((2.0f * far_clip * near_clip) / (far_clip - near_clip));
		return result;
	}

	internal inline
	m4x4 multiply(const m4x4& a, const m4x4& b)
	{
		m4x4 result = {0.0f};

		const f32* a_p = &a.elements[0];
		const f32* b_p = &b.elements[0];
		f32* dst_p = &result.elements[0];

		for(i8 i=0; i<4; ++i)
		{
			for(i8 j=0; j<4; ++j)
			{
				*dst_p =
					a_p[0]*b_p[0+j] +
					a_p[1]*b_p[4+j] +
					a_p[2]*b_p[8+j] +
					a_p[3]*b_p[12+j];
				++dst_p;
			}
			a_p+=4;
		}
		return result;
	}
};

internal inline
v3& x_axis(m4x4& m)
{
	return *(v3*)&m.elements[0];
}

internal inline
v3& y_axis(m4x4& m)
{
	return *(v3*)&m.elements[4];
}

internal inline
v3& z_axis(m4x4& m)
{
	return *(v3*)&m.elements[8];
}

internal inline
v3& translation(m4x4& m)
{
	return *(v3*)&m.elements[12];
}

inline
m4x4 operator*(const m4x4& a, const m4x4& b)
{
	m4x4 result = {0.0f};

	const f32* a_p = &a.elements[0];
	const f32* b_p = &b.elements[0];
	f32* dst_p = &result.elements[0];

	for(i8 i=0; i<4; ++i)
	{
		for(i8 j=0; j<4; ++j)
		{
			*dst_p =
				a_p[0]*b_p[0+j] +
				a_p[1]*b_p[4+j] +
				a_p[2]*b_p[8+j] +
				a_p[3]*b_p[12+j];
			++dst_p;
		}
		a_p+=4;
	}
	return result;
}

inline void
operator*=(m4x4& a, const m4x4& b)
{
	a = a*b;
}