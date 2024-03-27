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
};

inline
m4x4 operator*(m4x4& a, m4x4& b)
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

internal inline
v3 multiply_matrix_vector(m4x4& m, v3& p)
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
}

template<u8 rows, u8 columns>
struct matrix_base
{
	static_assert(rows>0, "matrix has to have atleast 1 row.");
	static_assert(columns>0, "matrix has to have atleast 1 column.");
	internal constexpr u8 rows = rows;
	internal constexpr u8 columns = columns;

	f32 elements[rows][columns];

	inline f32* operator[](i32 i)
	{
		assert(i>=0 && i<rows);
		return elements[i];
	}

	internal inline constexpr
	matrix_base<rows, columns> identity()
	{
		static_assert(rows == columns, "identity only available when rows and columns are equal.");
		matrix_base<rows, columns> result = {};
		for(u8 row = 0; row<rows; ++row)
		{
			for(u8 column = 0; column<columns; ++column)
			{
				result[row][column] = (row==column) ? 1.0f : 0.0f;
			}
		}
		return result;
	}
};

template<typename m1, typename m2>
inline matrix_base<m1::rows, m2::columns> operator*(m1 a, m2 b)
{
	static_assert(m1::columns == m2::rows, "Matrix A columns must equal to Matrix B rows.");
	matrix_base<m1::rows, m2::columns> result = {};
	for(u8 row=0; row<m1::rows; ++row)
	{
		for(u8 column=0; column<m2::columns; ++column)
		{
			result[row][column] = 0.0f;
			for(u8 i=0; i<m1::columns; ++i)
			{
				result[row][column] += a[row][i] * b[i][column];
			}
		}
	}
	return result;
}

typedef matrix_base<4, 1> m4x1;