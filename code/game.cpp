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

					v->elements[element_i] = ((f32)value / (f32)div)*(f32)sign;
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

internal inline void
_clamp_point_to_screen(render_output& output, v3& point, v3& other_point)
{
	if(point.x < 0.0f)
	{
		f32 x_offset = math::abs(point.x);
		f32 t = x_offset / math::abs(other_point.x-point.x);
		f32 y = math::lerp(point.y, other_point.y, t);
		point.x = 0.0f;
		point.y = y;
	}
	if(point.y < 0.0f)
	{
		f32 y_offset = math::abs(point.y);
		f32 t = y_offset / math::abs(other_point.y-point.y);
		f32 x = math::lerp(point.x, other_point.x, t);
		point.y = 0.0f;
		point.x = x;
	}
	if(point.x > (f32)output.width)
	{
		f32 x_offset = point.x - (f32)output.width;
		f32 t = x_offset / math::abs(other_point.x-point.x);
		f32 y = math::lerp(point.y, other_point.y, t);
		point.x = (f32)output.width;
		point.y = y;
	}
	if(point.y > (f32)output.height)
	{
		f32 y_offset = point.y - (f32)output.height;
		f32 t = y_offset / math::abs(other_point.y-point.y);
		f32 x = math::lerp(point.x, other_point.x, t);
		point.y = (f32)output.height;
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

		game_state->camera_position = {0.0f, -5.0f, 0.0f};
		game_state->camera_direction = {0.0f, 1.0f, 0.0f};

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

	f32 x_rot = PI*0.25f;//game_state->rotation;
	m4x4 rotation_x = m4x4::make_rotation_x(0.0f);

	f32 y_rot = game_state->rotation;
	m4x4 rotation_y = m4x4::make_rotation_y(0.0f);

	f32 z_rot = PI*0.25f + game_state->rotation;//game_state->rotation;
	m4x4 rotation_z = m4x4::make_rotation_z(z_rot);

	v3 box_position = {0.0f, 0.0f, 0.0f};
	m4x4 translation = m4x4::make_translation(box_position);
	m4x4 box_world_matrix = translation*rotation_z*rotation_y*rotation_x;
	// m4x4 box_world_matrix = translation;

	v3 up = {0.0f, 0.0f, 1.0f};
	game_state->camera_direction = {0.0f, 1.0f, 0.0f};

	v3 camera_move_direction = {0.0f, 0.0f, 0.0f};

	if(input::button_held(Q))
	{
		camera_move_direction -= {0.0f, 0.0f, 1.0f};
	}
	if(input::button_held(E))
	{
		camera_move_direction += {0.0f, 0.0f, 1.0f};
	}
	if(input::button_held(A))
	{
		camera_move_direction -= {1.0f, 0.0f, 0.0f};
	}
	if(input::button_held(D))
	{
		camera_move_direction += {1.0f, 0.0f, 0.0f};
	}
	if(input::button_held(W))
	{
		camera_move_direction += game_state->camera_direction;
	}
	if(input::button_held(S))
	{
		camera_move_direction -= game_state->camera_direction;
	}
	if(length_squared(camera_move_direction) != 0.0f)
	{
		camera_move_direction = normalize(camera_move_direction);
		game_state->camera_position += camera_move_direction*1.0f*time.dt;
	}

	// m4x4 camera_matrix = m4x4::look_at(game_state->camera_position,
	// 	game_state->camera_direction, up);
	// m4x4 view_matrix = m4x4::quick_inverse(camera_matrix);

	local_persist v3 light_direction = normalize({0.0f, -1.0f, 0.0f});

	// clear depth buffer
	f32* depth_buffer = (f32*)((u8*)render_output.memory + render_output.pitch*render_output.height);
	for(i32 y=0; y<render_output.height; ++y)
	{
		for(i32 x=0; x<render_output.width; ++x)
		{
			*(depth_buffer + y*render_output.width + x) = math::huge;
		}
	}

	f32 near = 0.1f;
	f32 far = 1000.0f;
	f32 vertical_fov = 120.0f;
	f32 aspect_ratio = (f32)render_output.width/(f32)render_output.height;
	f32 fov_rad = 1.0f / math::tangent(vertical_fov*math::deg_to_rad * 0.5f);

	m4x4 perspective_projection = {};
	perspective_projection[0][0] = 1.0f / (aspect_ratio*fov_rad);
	perspective_projection[1][1] = far / (far-near);
	perspective_projection[1][3] = -(far*near) / (far-near);
	perspective_projection[2][2] = 1.0f / fov_rad;
	perspective_projection[3][1] = 1.0f;

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

	m4x4 model_view;
	{
		v3 y = game_state->camera_direction;
		v3 x = cross(up, y);
		v3 z = cross(y, x);
		m4x4 camera_rotation = m4x4::identity();
		m4x4 camera_translation = m4x4::identity();
		for(int i=0; i<3; ++i)
		{
			camera_rotation[0][i] = x.elements[i];
			camera_rotation[1][i] = y.elements[i];
			camera_rotation[2][i] = z.elements[i];
			camera_translation[i][3] = -game_state->camera_position.elements[i];
		}
		model_view = camera_translation*camera_rotation;
	}

	// loaded_mesh& mesh = game_state->test_mesh;
	loaded_mesh& mesh = box_mesh;
	u32 num_discarded_triangles = 0;
	u32 num_rendered_triangles = 0;
	for(i32 face_i=0; face_i<mesh.num_faces; ++face_i)
	{
		triangle& tri = mesh.faces[face_i];

		v3 screen_pos[3];
		v3 camera_rel_positions[3];
		v3 world_positions[3];

		bool should_clip = true;
		for(i32 i=0; i<3; ++i)
		{
			v3 local_position = mesh.vertices[tri.indicies[i]];
			v3 world_position = m4x4::multiply_vector(box_world_matrix, local_position);
			v3 camera_rel_position = m4x4::multiply_vector(model_view, world_position);
			v3 projected_pos = m4x4::multiply_vector(perspective_projection, camera_rel_position);
			
			should_clip &= (projected_pos.x < -1.0f || projected_pos.x > 1.0f) ||
							(projected_pos.y < -1.0f || projected_pos.y > 1.0f) ||
							(projected_pos.z < -1.0f || projected_pos.z > 1.0f);
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

		v3 normal = normalize(cross(camera_rel_positions[1]-camera_rel_positions[0], camera_rel_positions[2]-camera_rel_positions[0]));
		if(dot(camera_rel_positions[0], normal) < 0.0f)
		{
			++num_discarded_triangles;
			continue;
		}

		v3 world_normal = normalize(cross(world_positions[1]-world_positions[0], world_positions[2]-world_positions[0]));
		f32 intensity = math::max(dot(world_normal, light_direction), 0.0f);
		f32 r = 0.9f;
		f32 g = 0.1f;
		f32 b = 0.2f;
		_draw_triangle(&render_output, screen_pos, r*intensity, g*intensity, b*intensity);

		// _draw_line(render_output, screen_pos[0], screen_pos[1], 0.0f, 0.0f, 0.0f);
		// _draw_line(render_output, screen_pos[0], screen_pos[2], 0.0f, 0.0f, 0.0f);
		// _draw_line(render_output, screen_pos[1], screen_pos[2], 0.0f, 0.0f, 0.0f);
		++num_rendered_triangles;
	}

	i32 asd = 0;
}

extern "C" GAME_FILL_AUDIO_OUTPUT(fill_audio_output)
{
	game_state* game_state = (game::game_state*)memory->permanent_storage;
}
}//namespace game