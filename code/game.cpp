#include "game.h"
#include "game_input.cpp"
#include "2d_exploration.cpp"

namespace game
{

internal inline
loaded_mesh _debug_load_mesh(thread_context* thread, debug_read_entire_file_def* read_entire_file,
	debug_free_file_memory_def* free_file_memory, memory_space& mesh_memory, char* file_name)
{
	loaded_mesh result = {};
	debug_read_file_results read_result = read_entire_file(thread, file_name);
	if(!read_result.content)
	{
		result.vertices = nullptr;
		result.faces = nullptr;
		result.num_vertices = 0;
		result.num_faces = 0;
		return result;
	}

	result.vertices = nullptr;
	result.faces = nullptr;

	memory_index file_index=0;
	const char* start = (const char*)read_result.content;
	while(file_index<read_result.size)
	{
		constexpr u16 MAX_LINES = 512;
		char line[MAX_LINES];
		u16 line_size = 0;
		for(u16 line_index = 0; line_index < MAX_LINES; ++line_index)
		{
			const char* current = start + file_index;
			line[line_index] = *current;
			++file_index;
			if(line[line_index] == '\n')
			{
				line_size = line_index+1;
				break;
			}
		}

		if(line_size != 0)
		{
			if(line[0] == 'v')
			{
				v3* v = _push_struct(&mesh_memory, v3);
				f32* v_p = (f32*)v;
				++result.num_vertices;
				if(result.vertices == nullptr)
				{
					result.vertices = v;
				}
				
				u16 line_index = 1;
				for(u8 element_i=0; element_i<3; ++element_i)
				{
					i32 value = 0;
					i32 div = 1;
					i32 sign = 1;

					bool found_start = false;
					for(i32 seen_point = 0; line_index<line_size; ++line_index)
					{
						if(line[line_index] == '.')
						{
							seen_point = 1;
							continue;
						}
						if(line[line_index] == '-')
						{
							sign = -1;
							continue;
						}

						i32 d = line[line_index] - '0';						
						if(d>=0 && d<=9)
						{
							found_start = true;

							if(seen_point)
							{
								div *= 10;
							}
							value = value*10 + d;
						}
						else if(found_start)
						{
							break;
						}
					}

					v_p[element_i] = ((f32)value / (f32)div)*(f32)sign;
				}
			}
			else if(line[0] == 'f')
			{
				triangle* f = _push_struct(&mesh_memory, triangle);
				++result.num_faces;
				if(result.faces == nullptr)
				{
					result.faces = f;
				}

				u16 line_index = 1;
				for(u8 index = 0; index<3; ++index)
				{
					i32 value = 0;
					bool found_start = false;
					for(; line_index<line_size; ++line_index)
					{
						i32 d = line[line_index] - '0';	
						if(d>=0 && d<=9)
						{
							found_start = true;
							value = value*10 + d;
						}
						else if(found_start)
						{
							break;
						}
					}
					f->indicies[index] = value-1;
				}
			}
		}
	}

	free_file_memory(thread, read_result.content);

	return result;
}

internal inline
v3 _barycentric(math::v2& a, math::v2& b, math::v2& c, math::v2& p)
{
	v3 result;
	v3 u = cross({c.x-a.x, b.x-a.x, a.x-p.x}, {c.y-a.y, b.y-a.y, a.y-p.y});
	if(math::abs(u.z)<1.0f)
	{
		result = {-1.0f, 1.0f, 1.0f};
	}
	else
	{
		result = {
			1.0f - (u.x+u.y)/u.z,
			u.y/u.z,
			u.x/u.z
		};
	}
	return result;
}

internal
void _draw_triangle(game::render_output* render_output, v3* points, f32 red, f32 green, f32 blue)
{
	math::v2 a = {points[0].x, points[0].y};
	math::v2 b = {points[1].x, points[1].y};
	math::v2 c = {points[2].x, points[2].y};
	f32 triangle_area = math::cross(b-a, c-a)*0.5f;
	i32 min_x = math::round_f32_to_i32(math::min(math::min(a.x, b.x), c.x));
	i32 max_x = math::round_f32_to_i32(math::max(math::max(a.x, b.x), c.x));
	i32 min_y = math::round_f32_to_i32(math::min(math::min(a.y, b.y), c.y));
	i32 max_y = math::round_f32_to_i32(math::max(math::max(a.y, b.y), c.y));

	if(min_x < 0)
	{
		min_x = 0;
	}
	if(min_y < 0)
	{
		min_y = 0;
	}
	if(max_x >= render_output->width)
	{
		max_x = render_output->width-1;
	}
	if(max_y >= render_output->height)
	{
		max_y = render_output->height-1;
	}

	u32 color = (math::round_f32_to_u32(red*255.0f)<<16) |
		(math::round_f32_to_u32(green*255.0f)<<8) |
		math::round_f32_to_u32(blue*255.0f);

	f32* depth_buffer = (f32*)((u8*)render_output->memory + render_output->height*render_output->pitch);

	u8* row = (u8*)render_output->memory + min_y*render_output->pitch;
	for(i32 y=min_y; y<=max_y; y++)
	{
		for(i32 x=min_x; x<=max_x; ++x)
		{
			math::v2 p = {(f32)x, (f32)y};
			v3 bc_screen = _barycentric(a, b, c, p);
			if(!(bc_screen.x<0.0f || bc_screen.y<0.0f || bc_screen.z<0.0f))
			{
				f32 depth = points[0].z*bc_screen.x + points[1].z*bc_screen.y + points[2].z*bc_screen.z;
				f32* saved_depth = depth_buffer + y*render_output->width + x;
				if(depth <= *saved_depth)
				{
					*saved_depth = depth;
					*((u32*)row+x) = color;
				}
			}
		}
		row += render_output->pitch;
	}
}

internal inline f32
_clamp_point_t_safe(f32 value, f32 diff)
{
	f32 div = diff == 0.0f ? 1.0f : math::abs(diff);
	f32 result = value / div;
	assert((result != result) == false);
	return result;
}

internal inline void
_clamp_point_to_screen(render_output& output, v3& point, v3& other_point)
{
	if(point.x < 0.0f)
	{
		f32 t = _clamp_point_t_safe(math::abs(point.x), other_point.x-point.x);
		assert((t != t) == false);
		// f32 t = x_offset / math::abs(other_point.x-point.x);
		f32 y = math::lerp(point.y, other_point.y, t);
		point.x = 0.0f;
		point.y = y;
	}
	if(point.y < 0.0f)
	{
		f32 t = _clamp_point_t_safe(math::abs(point.y), other_point.y-point.y);
		assert((t != t) == false);
		// f32 t = y_offset / math::abs(other_point.y-point.y);
		f32 x = math::lerp(point.x, other_point.x, t);
		point.y = 0.0f;
		point.x = x;
	}
	f32 max_width = (f32)(output.width-1);
	if(point.x > max_width)
	{
		f32 t = _clamp_point_t_safe(point.x-max_width, other_point.x-point.x);
		assert((t != t) == false);
		// f32 t = x_offset / math::abs(other_point.x-point.x);
		f32 y = math::lerp(point.y, other_point.y, t);
		point.x = max_width;
		point.y = y;
	}
	f32 max_height = (f32)(output.height-1);
	if(point.y > max_height)
	{
		f32 t = _clamp_point_t_safe(point.y-max_height, other_point.y-point.y);
		assert((t != t) == false);
		// f32 t = y_offset / math::abs(other_point.y-point.y);
		f32 x = math::lerp(point.x, other_point.x, t);
		point.y = max_height;
		point.x = x;
	}
}

internal void
_draw_line(render_output& output, v3 p1, v3 p2, f32 red, f32 green, f32 blue)
{
	if((p1.x < 0.0f && p2.x < 0.0f) || (p1.x > (f32)output.width && p2.x > (f32)output.width) ||
		(p1.y < 0.0f && p2.y < 0.0f) || (p1.y > (f32)output.height && p2.y > (f32)output.height))
	{
		return;
	}

	_clamp_point_to_screen(output, p1, p2);
	_clamp_point_to_screen(output, p2, p1);

	i32 min_x = math::round_f32_to_i32(math::min(p1.x, p2.x));
	i32 max_x = math::round_f32_to_i32(math::max(p1.x, p2.x));
	i32 min_y = math::round_f32_to_i32(math::min(p1.y, p2.y));
	i32 max_y = math::round_f32_to_i32(math::max(p1.y, p2.y));

	i32 x_length = max_x-min_x;
	i32 y_length = max_y-min_y;

	u32 color = (math::round_f32_to_u32(red*255.0f)<<16) |
		(math::round_f32_to_u32(green*255.0f)<<8) |
		math::round_f32_to_u32(blue*255.0f);

	f32* depth_buffer = (f32*)((u8*)output.memory + output.height*output.pitch);

	if(x_length >= y_length)
	{
		v3 start, end;
		if(p1.x < p2.x)
		{
			start = p1;
			end = p2;
		}
		else
		{
			start = p2;
			end = p1;
		}

		f32 time_step = 1.0f/x_length;
		f32 t = 0.0f;
		for(i32 x=min_x; x<=max_x; ++x)
		{
			i32 y = math::round_f32_to_i32(math::lerp(start.y, end.y, t));
			f32 depth = math::lerp(start.z, end.z, t);
			f32* saved_depth = depth_buffer + y*output.width + x;
			if(depth <= *saved_depth)
			{
				*saved_depth = depth;
				*((u32*)((u8*)output.memory + y*output.pitch)+x) = color;
			}
			t += time_step;
		}
	}
	else
	{
		v3 start, end;
		if(p1.y < p2.y)
		{
			start = p1;
			end = p2;
		}
		else
		{
			start = p2;
			end = p1;
		}

		f32 time_step = 1.0f/y_length;
		f32 t = 0.0f;
		for(i32 y=min_y; y<=max_y; ++y)
		{
			i32 x = math::round_f32_to_i32(math::lerp(start.x, end.x, t));
			f32 depth = math::lerp(start.z, end.z, t);
			f32* saved_depth = depth_buffer + y*output.width + x;
			if(depth <= *saved_depth)
			{
				*saved_depth = depth;
				*((u32*)((u8*)output.memory + y*output.pitch)+x) = color;
			}
			t += time_step;
		}
	}
}

internal inline bool
_point_to_screen(const render_output& output, const v3& point, const m4x4& camera, const m4x4& perspective, v3* screen_position)
{
	bool should_clip = true;
	v3 camera_rel_position = m4x4::multiply(camera, point);
	v4 homo_pos = m4x4::multiply(perspective, vector4(camera_rel_position, 1.0f));

	should_clip &= (homo_pos.x < -homo_pos.w || homo_pos.x > homo_pos.w) ||
					(homo_pos.y < 0.0f || homo_pos.y > homo_pos.w) ||
					(homo_pos.z < -homo_pos.w || homo_pos.z > homo_pos.w);

	v3 projected_pos = {
		homo_pos.x / homo_pos.w,
		homo_pos.y / homo_pos.w,
		homo_pos.z / homo_pos.w
	};

	*screen_position = {
		(projected_pos.x+1.0f) * 0.5f * (f32)output.width,
		(-projected_pos.z+1.0f) * 0.5f * (f32)output.height,
		projected_pos.y
	};

	return should_clip;
}

internal void
_draw_arrow(render_output& output, const v3& point, const v3& vector, const v4& color,
	const m4x4& camera_matrix, const m4x4& perspective)
{
	const v3 tip = point+vector;

	v3 tip_screen_position = {};
	_point_to_screen(output, tip, camera_matrix, perspective, &tip_screen_position);

	//arrow base
	{
		v3 base_screen_position = {};
		bool should_clip = true;
		should_clip &= _point_to_screen(output, point, camera_matrix, perspective, &base_screen_position);

		if(should_clip == false) {
			_draw_line(output, base_screen_position, tip_screen_position, color.x, color.y, color.z);
		}
	}

	const f32 tip_length = 0.1f;
	v3 up = {0.0f, 0.0f, 1.0f};
	if(math::abs(dot(vector, up)) == 1.0f) //todo(staffan): maybe better way of doing this?
	{
		up = {0.0f, 1.0f, 0.0f};
	}
	m4x4 arrow_look_at = m4x4::look_at(point, point+vector, up);
	v3 tip_base = point + vector*(1.0f-tip_length);
	{ //tip right
		v3 right_start = tip_base-x_axis(arrow_look_at)*tip_length;
		v3 right_end = tip_base+x_axis(arrow_look_at)*tip_length;
		v3 screen_positions[2];
		bool should_clip = true;
		should_clip &= _point_to_screen(output, right_start, camera_matrix, perspective, &screen_positions[0]);
		should_clip &= _point_to_screen(output, right_end, camera_matrix, perspective, &screen_positions[1]);

		if (should_clip == false) {
			_draw_line(output, screen_positions[0], tip_screen_position, color.x, color.y, color.z);
			_draw_line(output, screen_positions[1], tip_screen_position, color.x, color.y, color.z);
		}
	}

	{ //tip up		
		v3 up_start = tip_base-z_axis(arrow_look_at)*tip_length;
		v3 up_end = tip_base+z_axis(arrow_look_at)*tip_length;
		v3 screen_positions[2];
		bool should_clip = true;
		should_clip &= _point_to_screen(output, up_start, camera_matrix, perspective, &screen_positions[0]);
		should_clip &= _point_to_screen(output, up_end, camera_matrix, perspective, &screen_positions[1]);

		if (should_clip == false) {
			_draw_line(output, screen_positions[0], tip_screen_position, color.x, color.y, color.z);
			_draw_line(output, screen_positions[1], tip_screen_position, color.x, color.y, color.z);
		}
	}
}

extern "C" GAME_UPDATE_AND_RENDER(update_and_render)
{
	assert(memory->permanent_storage_size >= sizeof(game::game_state));
	game_state* game_state = (game::game_state*)memory->permanent_storage;

	if(!memory->is_initialized)
	{
#if GAME_INTERNAL
		if(memory->debug_goto_playback)
		{
			_goto_playback_func = memory->debug_goto_playback;
		}
#endif

		game_state->should_rotate = true;
		game_state->rotation = 0.0f;

		game_state->camera_position = {0.0f, -10.0f, 0.0f};
		game_state->camera_direction = {0.0f, 1.0f, 0.0f};
		game_state->camera_rotation = identity();

		u64 mesh_space_size = sizeof(v3)*10000 + sizeof(triangle)*10000;
		_initialize_memory_space(&game_state->mesh_space,
			(u8*)memory->permanent_storage+sizeof(game::game_state),
			mesh_space_size);

		_initialize_2d_exploration(thread, game_state, memory, sizeof(game::game_state)+mesh_space_size);

		game_state->test_mesh = _debug_load_mesh(thread, memory->debug_read_entire_file,
			memory->debug_free_file_memory, game_state->mesh_space, "test/monkey.obj");

		memory->is_initialized = true;
	}

	input::new_frame(&input);

	_update_2d_exploration(game_state, time, render_output);

	v3 box[8] = {
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, -0.5f,
	};

	triangle box_faces[12] = {
		0, 1, 2,
		0, 2, 3,
		3, 2, 5,
		3, 5, 4,
		4, 5, 6,
		4, 6, 7,
		7, 6, 1,
		7, 1, 0,
		1, 6, 5,
		1, 5, 2,
		7, 0, 3,
		7, 3, 4
	};

	loaded_mesh box_mesh = {};
	box_mesh.vertices = box;
	box_mesh.num_vertices = 8;
	box_mesh.faces = box_faces;
	box_mesh.num_faces = 12;

	if(input::button_pressed(MOUSE_RIGHT))
	{
		game_state->should_rotate = !game_state->should_rotate;
	}
	if(game_state->should_rotate)
	{
		game_state->rotation += time.dt;
		if(game_state->rotation >= TAU*2.0f)
		{
			game_state->rotation -= TAU*2.0f;
		}
	}

	m4x4 box_world_matrices[2];
	m4x4& box_world_matrix_1 = box_world_matrices[0];
	m4x4& box_world_matrix_2 = box_world_matrices[1];

	f32 x_rot = PI*0.25f;//game_state->rotation;
	m4x4 rotation_x = m4x4::make_rotation_x(0.0f);

	f32 y_rot = game_state->rotation;
	m4x4 rotation_y = m4x4::make_rotation_y(0.0f);

	f32 z_rot = PI*0.25f + game_state->rotation;//game_state->rotation;
	m4x4 rotation_z = m4x4::make_rotation_z(0.0f);

	v3 box_position = {0.0f, -5.0f, 0.0f};
	m4x4 box_translation = m4x4::make_translation(box_position);
	box_world_matrix_1 = box_translation*rotation_z*rotation_y*rotation_x;
	box_world_matrix_2 = m4x4::make_translation({-2.0f, 0.0f, 0.0f})*rotation_z*rotation_y*rotation_x;

	// m4x4 box_world_matrix = translation;

	v3 up = {0.0f, 0.0f, 1.0f};
	v3 camera_move_direction = {0.0f, 0.0f, 0.0f};

	{
		quaternion yaw_rotation = identity();
		{
			i32 yaw_direction = 0;
			if(input::button_held(A)) {
				yaw_direction += 1;
			}
			if(input::button_held(D)) {
				yaw_direction -= 1;
			}
			if(yaw_direction) {
				const_exp f32 yaw_speed = 45.0f*math::deg_to_rad;
				yaw_rotation = from_angle_axis(((f32)yaw_direction)*yaw_speed*time.dt, {0.0f, 0.0f, 1.0f});
			}
		}
		quaternion pitch_rotation = identity();
		{
			i32 pitch_direction = 0;
			if(input::button_held(W)) {
				pitch_direction -= 1;
			}
			if(input::button_held(S)) {
				pitch_direction += 1;
			}
			if(pitch_direction) {
				const_exp f32 pitch_speed = 45.0f*math::deg_to_rad;
				pitch_rotation = from_angle_axis(((f32)pitch_direction)*pitch_speed*time.dt, {1.0f, 0.0f, 0.0f});
			}
		}

		game_state->camera_rotation = normalize((pitch_rotation*yaw_rotation)*game_state->camera_rotation);

		v3 camera_forward = forward(game_state->camera_rotation);
		game_state->camera_position = -camera_forward * 10.0f;
	}

	// m4x4 camera_matrix = m4x4::look_at(game_state->camera_position,
	// 	game_state->camera_direction, up);
	// m4x4 view_matrix = m4x4::quick_inverse(camera_matrix);

	local_persist v3 light_direction = normalize(v3({-0.5f, 1.0f, -0.5f}));

	// clear depth buffer
	f32* depth_buffer = (f32*)((u8*)render_output.memory + render_output.pitch*render_output.height);
	for(i32 y=0; y<render_output.height; ++y)
	{
		for(i32 x=0; x<render_output.width; ++x)
		{
			*(depth_buffer + y*render_output.width + x) = math::huge;
		}
	}


	m4x4 perspective_projection = m4x4::identity();
	{
		f32 near = 0.3f;
		f32 far = 100.0f;

		v3 position = {-5.0f, (far-near)*0.5f, 1.0f};
		v3 n_pos = {-5.0f, near, 1.0f};
		v3 f_pos = {-5.0f, far, 1.0f};

		f32 horizontal_fov = 90.0f;
		f32 aspect_ratio = (f32)render_output.width/(f32)render_output.height;
		f32 fov_rad = 1.0f / math::tangent(horizontal_fov*math::deg_to_rad * 0.5f);

		f32 tangent = math::tangent(horizontal_fov*0.5f * math::deg_to_rad);
		f32 right = near*tangent;
		f32 top = right/aspect_ratio;
		f32 depth = far-near;

		f32* p = &perspective_projection.elements[0];
		p[0] = near / right;
		p[5] = far / depth;
		p[7] = 1.0f;
		p[10] = near / top;
		p[13] = (-(far*near)) / depth;
		// p[5] = -((far+near) / (far-near));
		// p[7] = -((2.0f*far*near) / (far-near));
		// p[10] = 1.0f / math::tangent(vertical_fov*0.5f * math::deg_to_rad);
		p[15] = 0.0f;

		//opengl perspective
		// f32 tangent = math::tangent(vertical_fov*0.5f * math::deg_to_rad);
		// f32 top = near*tangent;
		// f32 right = top*aspect_ratio;
		// f32 depth = far-near;

		// f32* p = &perspective_projection.elements[0];
		// p[0] = near / right;
		// p[5] = near / top;
		// p[10] = -(far+near) / depth;
		// p[11] = -1.0f;
		// p[14] = -(2.0f*far*near) / depth;
		// p[15] = 0.0f;


		v3 test = m4x4::multiply_vector(perspective_projection, position);
		v4 test2 = m4x4::multiply(perspective_projection, vector4(position, 1.0f));
		v3 test3 = {
			test2.x/test2.w,
			test2.y/test2.w,
			test2.z/test2.w,
		};

		v4 n_test = m4x4::multiply(perspective_projection, vector4(n_pos, 1.0f));
		v3 n_test1 = {
			n_test.x/n_test.w,
			n_test.y/n_test.w,
			n_test.z/n_test.w,
		};

		v4 f_test = m4x4::multiply(perspective_projection, vector4(f_pos, 1.0f));
		v3 f_test1 = {
			f_test.x/f_test.w,
			f_test.y/f_test.w,
			f_test.z/f_test.w,
		};

		int asd = 0;
	}


	// f32 near = 0.1f;
	// f32 far = 1000.0f;
	// f32 vertical_fov = 120.0f;
	// f32 aspect_ratio = (f32)render_output.width/(f32)render_output.height;
	// f32 fov_rad = 1.0f / math::tangent(vertical_fov*math::deg_to_rad * 0.5f);

	// m4x4 perspective_projection = m4x4::perspective(fov_rad, aspect_ratio, near, far);

/*
	m4x4 projection = m4x4::identity();
	projection[3][1] = -1.0f/game_state->camera_position.y;
*/

/*
	m4x4 viewport = m4x4::identity();
	{
		viewport[0][0] = (f32)render_output.width/2.0f;
		viewport[1][1] = 255.0f/2.0f;
		viewport[2][2] = (f32)render_output.height/2.0f;
		viewport[3][0] = (f32)render_output.width + (f32)render_output.width/2.0f;
		viewport[3][1] = 255.0f/2.0f;
		viewport[3][2] = (f32)render_output.height + (f32)render_output.height/2.0f;
	}
*/

	m4x4 model_view = m4x4::identity();
	{
		v3 y = game_state->camera_direction;
		v3 x = cross(y, up);
		v3 z = cross(x, y);
		f32* x_p = (f32*)&x;
		f32* y_p = (f32*)&y;
		f32* z_p = (f32*)&z;
		m4x4 camera_translation = m4x4::identity();
		f32* rotation_ptr = &camera_translation.elements[0];
		f32* translation_ptr = &camera_translation.elements[12];
		f32* camera_position = (f32*)&game_state->camera_position;
		for(int i=0; i<3; ++i)
		{
			rotation_ptr[0+i] = x_p[i];
			rotation_ptr[4+i] = y_p[i];
			rotation_ptr[8+i] = z_p[i];
			translation_ptr[i] = camera_position[i];
		}
		// model_view = m4x4::quick_inverse(camera_translation);


		m4x4 camera_transform = matrix(game_state->camera_rotation);
		translation(camera_transform) = game_state->camera_position;
		model_view = m4x4::quick_inverse(camera_transform);

		// model_view = m4x4::identity();
		// f32* model_view_ptr = &model_view.elements[0];
		// for(int i=0; i<3; ++i)
		// {
		// 	model_view_ptr[0+i] = x.elements[i];
		// 	model_view_ptr[4+i] = y.elements[i];
		// 	model_view_ptr[8+i] = z.elements[i];
		// 	model_view_ptr[12+i] = -game_state->camera_position.elements[i];
		// }

		// for(int i=0; i<3; ++i)
		// {
		// 	rotation_ptr[0] = x.elements[i];
		// 	rotation_ptr[1] = y.elements[i];
		// 	rotation_ptr[2] = z.elements[i];
		// 	translation_ptr[3] = -game_state->camera_position.elements[i];
		// 	rotation_ptr+=4;
		// 	translation_ptr+=4;
		// }
		// model_view = camera_translation*camera_rotation;

		// for(int i=0; i<3; ++i)
		// {
		// 	camera_rotation[i][0] = x.elements[i];
		// 	camera_rotation[i][1] = y.elements[i];
		// 	camera_rotation[i][2] = z.elements[i];
		// 	camera_translation[i][3] = -game_state->camera_position.elements[i];
		// }
		// model_view = camera_translation*camera_rotation;
	}

	// loaded_mesh& mesh = game_state->test_mesh;
	loaded_mesh& mesh = box_mesh;
	u32 num_discarded_triangles = 0;
	u32 num_rendered_triangles = 0;
	for(u32 transform_i=0; transform_i<array_size(box_world_matrices); ++transform_i)
	{
		m4x4& box_world_matrix = box_world_matrices[transform_i];
		m4x4 test = perspective_projection*model_view*box_world_matrix;
		for(i32 face_i=0; face_i<mesh.num_faces; ++face_i)
		{
			triangle& tri = mesh.faces[face_i];

			v3 screen_pos[3];
			v3 camera_rel_positions[3];
			v3 world_positions[3];

			{
				bool should_clip = true;
				for(i32 i=0; i<3; ++i)
				{
					v3& local_position = mesh.vertices[tri.indicies[i]];
					v3 world_position = m4x4::multiply(box_world_matrix, local_position);
					v3 camera_rel_position = m4x4::multiply(model_view, world_position);
					v4 homogenous_pos = m4x4::multiply(perspective_projection, vector4(camera_rel_position, 1.0f));

					should_clip &= (homogenous_pos.x < -homogenous_pos.w || homogenous_pos.x > homogenous_pos.w) ||
									(homogenous_pos.y < 0.0f || homogenous_pos.y > homogenous_pos.w) ||
									(homogenous_pos.z < -homogenous_pos.w || homogenous_pos.z > homogenous_pos.w);

					v3 projected_pos = {
						homogenous_pos.x / homogenous_pos.w,
						homogenous_pos.y / homogenous_pos.w,
						homogenous_pos.z / homogenous_pos.w
					};
					
					screen_pos[i] = {
						(projected_pos.x+1.0f) * 0.5f * (f32)render_output.width,
						(-projected_pos.z+1.0f) * 0.5f * (f32)render_output.height,
						projected_pos.y
					};

					camera_rel_positions[i] = camera_rel_position;
					world_positions[i] = world_position;
				}

				if(should_clip)
				{
					++num_discarded_triangles;
					continue;
				}
			}

			v3 normal = normalize(cross(camera_rel_positions[1]-camera_rel_positions[0], camera_rel_positions[2]-camera_rel_positions[0]));
			if(dot(camera_rel_positions[0], normal) > 0.0f)
			{
				++num_discarded_triangles;
				continue;
			}

			v3 world_normal = normalize(cross(world_positions[1]-world_positions[0], world_positions[2]-world_positions[0]));
			f32 intensity = math::max(-1.0f*dot(world_normal, light_direction), 0.05f);
			f32 r = 0.7f;
			f32 g = 0.1f;
			f32 b = 0.2f;
			_draw_triangle(&render_output, screen_pos, r*intensity, g*intensity, b*intensity);


			const_exp bool draw_normals = true;
			if (draw_normals) {
				v3 from = (world_positions[0]+world_positions[1]+world_positions[2]) / 3;
				v4 color = {0.1f, 0.45f, 0.45f, 1.0f};
				_draw_arrow(render_output, from, world_normal, color, model_view, perspective_projection);
			}

			// _draw_line(render_output, screen_pos[0], screen_pos[1], 0.0f, 0.0f, 0.0f);
			// _draw_line(render_output, screen_pos[0], screen_pos[2], 0.0f, 0.0f, 0.0f);
			// _draw_line(render_output, screen_pos[1], screen_pos[2], 0.0f, 0.0f, 0.0f);
			++num_rendered_triangles;
		}
	}

	i32 asd = 0;
}

extern "C" GAME_FILL_AUDIO_OUTPUT(fill_audio_output)
{
	game_state* game_state = (game::game_state*)memory->permanent_storage;
}
}//namespace game