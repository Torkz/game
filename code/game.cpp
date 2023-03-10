#include "game.h"
#include "game_intrinsics.cpp"
#include "game_input.cpp"
#include "game_tilemap.cpp"

namespace game
{

internal void _draw_rectangle(render_output& output, math::vector2 min, math::vector2 max, float32 red, float32 green, float32 blue)
{
	int min_x_rounded = math::round_float_to_uint32(min.x);
	int min_y_rounded = math::round_float_to_uint32(min.y);
	int max_x_rounded = math::round_float_to_uint32(max.x);
	int max_y_rounded = math::round_float_to_uint32(max.y);

	if(min_x_rounded < 0)
	{
		min_x_rounded = 0;
	}

	if(min_y_rounded < 0)
	{
		min_y_rounded = 0;
	}

	if(max_x_rounded > output.width)
	{
		max_x_rounded = output.width;
	}

	if(max_y_rounded > output.height)
	{
		max_y_rounded = output.height;
	}

	uint32 color =
		(math::round_float_to_uint32(red*255.0f)<<16) |
		(math::round_float_to_uint32(green*255.0f)<<8) |
		math::round_float_to_uint32(blue*255.0f);

	uint8* row = (uint8*)output.memory+(min_y_rounded*output.pitch);
	for(int y=min_y_rounded; y<max_y_rounded; ++y)
	{
		for(int x=min_x_rounded; x<max_x_rounded; ++x)
		{		
			*((uint32*)row+x) = color;
		}
		row += output.pitch;
	}
}

internal void
_draw_bitmap(render_output* output, loaded_bitmap* bitmap, math::vector2 start)
{
	int blit_width = bitmap->width;
	int blit_height = bitmap->height;

	int rounded_start_x = math::round_float_to_uint32(start.x) + bitmap->offset_x;
	int rounded_start_y = math::round_float_to_uint32(start.y) + bitmap->offset_y;

	uint32* source_row = bitmap->pixels + bitmap->width*(bitmap->height-1);

	if(rounded_start_x < 0)
	{
		source_row -= rounded_start_x; 
		blit_width += rounded_start_x;
		rounded_start_x = 0;
	}

	if(rounded_start_y < 0)
	{
		source_row += rounded_start_y*bitmap->width;
		blit_height += rounded_start_y;
		rounded_start_y = 0;
	}

	if((rounded_start_x+blit_width) > output->width)
	{
		blit_width -= (rounded_start_x+blit_width)-output->width;
	}

	if((rounded_start_y+blit_height) > output->height)
	{
		blit_height -= (rounded_start_y+blit_height)-output->height;
	}

	if(blit_width <= 0 || blit_height <= 0)
	{
		return;
	}

	uint8* dest_row = (uint8*)((uint32*)((uint8*)output->memory + rounded_start_y*output->pitch) + rounded_start_x);
	for(int y=0;
		y<blit_height;
		++y)
	{
		for(int x=0;
			x<blit_width;
			++x)
		{
			uint32* source = source_row+x;
			uint32* dest = (uint32*)dest_row+x;

			if(*source & 0xFF000000)
			{
				*dest = *source;
			}
		}
		source_row -= bitmap->width;
		dest_row += output->pitch;
	}
}

internal
void _initialize_memory_space(memory_space* space, uint8* base, memory_index max_memory)
{
	space->base = base;
	space->max_memory = max_memory;
	space->used_memory = 0;
}

#pragma pack(push, 1)
struct bmp_header
{
	uint16 file_type;     /* File type, always 4D42h ("BM") */
	uint32 file_size;     /* Size of the file in bytes */
	uint16 reserved_1;    /* Always 0 */
	uint16 reserved_2;    /* Always 0 */
	uint32 bitmap_offset; /* Starting position of image data in bytes */
	uint32 header_size;   /* Size of this header in bytes */
	int32 width;           /* Image width in pixels */
	int32 height;          /* Image height in pixels */
	uint16 planes;          /* Number of color planes */
	uint16 bits_per_pixel;    /* Number of bits per pixel */
	uint32 compression;     /* Compression methods used */
	uint32 size_of_bitmap;    /* Size of bitmap in bytes */
	int32 horzizontal_resolution;  /* Horizontal resolution in pixels per meter */
	int32 vertical_resolution;  /* Vertical resolution in pixels per meter */
	uint32 colors_used;      /* Number of colors in the image */
	uint32 colors_important; /* Minimum number of important colors */

	uint32 red_mask;         /* Mask identifying bits of red component */
	uint32 green_mask;       /* Mask identifying bits of green component */
	uint32 blue_mask;        /* Mask identifying bits of blue component */
};
#pragma pack(pop)

internal
loaded_bitmap _debug_load_bmp(thread_context* thread, debug_read_entire_file_def* read_entire_file, char* file_name)
{
	loaded_bitmap result = {};
	debug_read_file_results read_result = read_entire_file(thread, file_name);
	if(!read_result.content)
	{
		result.pixels = nullptr;
		return result;
	}

	bmp_header* header = (bmp_header*)read_result.content;
	result.pixels = (uint32*)((uint8*)read_result.content + header->bitmap_offset);
	result.width = header->width;
	result.height = header->height;

	assert(header->compression == 3);

	uint32 red_mask = header->red_mask;
	uint32 green_mask = header->green_mask;
	uint32 blue_mask = header->blue_mask;
	uint32 alpha_mask = ~(red_mask | green_mask | blue_mask);

	using namespace bit;
	scan_bit_result red_shift = find_least_significant_set_bit(red_mask);
	scan_bit_result green_shift = find_least_significant_set_bit(green_mask);
	scan_bit_result blue_shift = find_least_significant_set_bit(blue_mask);
	scan_bit_result alpha_shift = find_least_significant_set_bit(alpha_mask);
	assert(red_shift.found && green_shift.found && blue_shift.found && alpha_shift.found);

	uint32* source_dest = result.pixels;
	for(int32 i=0;
		i<header->height*header->width;
		++i)
	{
		*source_dest = (((*source_dest >> alpha_shift.index) & 0xFF) << 24) |
						(((*source_dest >> red_shift.index) & 0xFF) << 16) |
						(((*source_dest >> green_shift.index) & 0xFF) << 8) |
						((*source_dest >> blue_shift.index) & 0xFF);
		++source_dest;
	}

	return result;
}

extern "C" GAME_UPDATE_AND_RENDER(update_and_render)
{
	assert(memory->permanent_storage_size >= sizeof(game::game_state));
	game_state* game_state = (game::game_state*)memory->permanent_storage;

	float32 player_height = 1.8f;
	float32 player_width = player_height*0.75f;

	if(!memory->is_initialized)
	{
		game_state->test_bitmap = _debug_load_bmp(thread, memory->debug_read_entire_file, "test/test_background.bmp");
		game_state->test_bitmap.offset_x = 0;
		game_state->test_bitmap.offset_y = 0;
		game_state->player_bitmap = _debug_load_bmp(thread, memory->debug_read_entire_file, "test/character01.bmp");
		game_state->player_bitmap.offset_x = -32;
		game_state->player_bitmap.offset_y = -59;

		game_state->player_position.tile_x = 3;
		game_state->player_position.tile_y = 3;
		game_state->player_position.tile_z = 0;
		game_state->player_position.tile_relative = {0.0f, 0.0f};

		_initialize_memory_space(&game_state->world_space,
			(uint8*)memory->permanent_storage+sizeof(game::game_state),
			memory->permanent_storage_size-sizeof(game::game_state)
		);
		
		game_state->world = _push_struct(&game_state->world_space, game::world);
		world* world = game_state->world;

		world->tile_map = _push_struct(&game_state->world_space, game::tile_map);
		tile_map* tile_map = world->tile_map;

		tile_map->memory_space = &game_state->world_space;

		tile_map->chunk_shift = 4;
		tile_map->chunk_dimension = (1 << tile_map->chunk_shift);
		tile_map->chunk_mask = tile_map->chunk_dimension - 1;

		tile_map->num_chunks_x = 128;
		tile_map->num_chunks_y = 128;
		tile_map->num_chunks_z = 128;
		tile_map->chunks = _push_array(&game_state->world_space, game::chunk,
										tile_map->num_chunks_x*
										tile_map->num_chunks_y*
										tile_map->num_chunks_z);

		tile_map->tile_side_in_metres = 2.0f;

		uint32 num_screen_tiles_x = 17;
		uint32 num_screen_tiles_y = 9;
		uint32 screen_x = 0;
		uint32 screen_y = 0;
		uint32 screen_z = 0;
		bool door_top = false;
		bool door_right = false;
		bool door_bottom = false;
		bool door_left = false;
		bool door_up = false;
		bool door_down = false;
		bool created_z_door = false;

		for(uint32 screen_index = 0;
			screen_index < 100;
			++screen_index)
		{
			if(screen_index != 99)
			{
				uint32 random = random_number(screen_index);

				if(created_z_door)
				{
					if(random%2 == 0)
					{
						door_top = true;
					}
					else
					{
						door_right = true;
					}
					created_z_door = false;
				}
				else
				{
					uint32 door_case = random%3;
					if(door_case == 0)
					{
						door_top = true;
					}
					else if(door_case == 1)
					{
						door_right = true;
					}
					else
					{
						created_z_door = true;
						if(screen_z == 0)
						{
							door_up = true;
						}
						else
						{
							door_down = true;
						}
					}
				}
			}

			for(uint32 y = 0;
				y < num_screen_tiles_y;
				++y)
			{
				for(uint32 x = 0;
					x < num_screen_tiles_x;
					++x)
				{
					uint32 tile_x = (screen_x*num_screen_tiles_x) + x;
					uint32 tile_y = (screen_y*num_screen_tiles_y) + y;

					uint32 tile_value = 1;
					if(x == 0)
					{
						if(y == (num_screen_tiles_y/2) && door_left)
						{
							tile_value = 1;
						}
						else
						{
							tile_value = 2;
						}
					}

					if(x == (num_screen_tiles_x-1))
					{
						if(y == (num_screen_tiles_y/2) && door_right)
						{
							tile_value = 1;
						}
						else
						{
							tile_value = 2;
						}
					}

					if(y == 0)
					{
						if(x == (num_screen_tiles_x/2) && door_bottom)
						{
							tile_value = 1;
						}
						else
						{
							tile_value = 2;
						}
					}

					if(y == (num_screen_tiles_y-1))
					{
						if(x == (num_screen_tiles_x/2) && door_top)
						{
							tile_value = 1;
						}
						else
						{
							tile_value = 2;
						}
					}

					if(x == 4 && y == 6)
					{
						if(door_up)
						{
							tile_value = 3;
						}
						if(door_down)
						{
							tile_value = 4;
						}
					}

					tiles::set_tile_value(tile_map, tile_x, tile_y, screen_z, tile_value);
				}
			}

			if(door_top)
			{
				++screen_y;
			}
			if(door_right)
			{
				++screen_x;
			}

			if(created_z_door)
			{
				if(door_up)
				{
					++screen_z;
				}
				else
				{
					--screen_z;
				}

				door_down = !door_down;
				door_up = !door_up;
			}
			else
			{
				door_up = false;
				door_down = false;
			}

			door_bottom = door_top;
			door_left = door_right;
			door_top = false;
			door_right = false;
		}

		memory->is_initialized = true;
	}

	world* world = game_state->world;
	tile_map* tile_map = world->tile_map;

	int tile_side_in_pixels = 60;
	float32 metres_to_pixels = tile_side_in_pixels/tile_map->tile_side_in_metres;

	input::new_frame(&input);

	math::vector2 delta = {};
	math::vector2 input_direction = {};

	//note(staffan): not really friction, just used when there's no input in that direction.
	math::vector2 friction_direction;
	if(math::length_squared(game_state->player_velocity) > 0.0f)
	{
		friction_direction = -math::normalize(game_state->player_velocity);
	}
	else
	{
		friction_direction = {0.0f, 0.0f};
	}

	if(input::button_held(A))
	{
		input_direction.x += -1.0f;
	}
	if(input::button_held(D))
	{
		input_direction.x += 1.0f;
	}
	if(input::button_held(W))
	{
		input_direction.y += 1.0f;
	}
	if(input::button_held(S))
	{
		input_direction.y += -1.0f;
	}

	bool wants_move = math::length_squared(input_direction) > 0.0f;
	if(wants_move)
	{
		input_direction = math::normalize(input_direction);

		if(input_direction.x != 0.0f)
		{
			friction_direction.x = 0.0f;
		}
		if(input_direction.y != 0.0f)
		{
			friction_direction.y = 0.0f;
		}
	}

	float32 max_speed = 7.0f;
	math::vector2 acceleration = input_direction*3.0f;
	math::vector2 friction = friction_direction*1.0f;
	math::vector2 wanted_velocity = game_state->player_velocity+friction;

	if(math::sign(wanted_velocity.x) != math::sign(game_state->player_velocity.x))
	{
		wanted_velocity.x = 0.0f;
	}
	if(math::sign(wanted_velocity.y) != math::sign(game_state->player_velocity.y))
	{
		wanted_velocity.y = 0.0f;
	}

	wanted_velocity += acceleration;

	if(wants_move && (math::length_squared(wanted_velocity) > max_speed*max_speed))
	{
		wanted_velocity = math::normalize(wanted_velocity)*max_speed;
	}

	float32 speed_scale = input::button_held(SPACEBAR) ? 5.0f : 1.0f;
	math::vector2 delta_this_frame = wanted_velocity*time.dt*speed_scale;

	tile_map_position new_position = game_state->player_position;
	new_position.tile_relative += delta_this_frame;
	new_position = tiles::recanonicalize_tile_map_position(tile_map, new_position);

	tile_map_position left_position = new_position;
	left_position.tile_relative.x -= player_width*0.5f;
	left_position = tiles::recanonicalize_tile_map_position(tile_map, left_position);

	tile_map_position right_position = new_position;
	right_position.tile_relative.x += player_width*0.5f;
	right_position = tiles::recanonicalize_tile_map_position(tile_map, right_position);

	bool position_is_valid = tiles::position_is_valid(tile_map, new_position)
							&& tiles::position_is_valid(tile_map, left_position)
							&& tiles::position_is_valid(tile_map, right_position);

	if(position_is_valid)
	{
		if(!tiles::are_on_same_tile(game_state->player_position, new_position))
		{
			uint32 tile_value = tiles::tile_value(tile_map, new_position);
			if(tile_value == 3)
			{
				++new_position.tile_z;
			}
			else if(tile_value == 4)
			{
				--new_position.tile_z;
			}
		}
		game_state->player_position = new_position;
		game_state->player_velocity = wanted_velocity;
	}
	else
	{
		game_state->player_velocity = {0.0f, 0.0f};
	}

	math::vector2 top_left = {0.0f, 0.0f};
	_draw_rectangle(render_output, {0.0f, 0.0f}, {(float32)render_output.width, (float32)render_output.height}, 1.0f, 0.0f, 1.0f);
	
	_draw_bitmap(&render_output, &game_state->test_bitmap, {0.0f, 0.0f});

	math::vector2 screen_center = {
		render_output.width*0.5f,
		render_output.height*0.5f
	};
	math::vector2 tile_side_half_extents_in_pixels = {
		(float32)tile_side_in_pixels*0.5f,
		(float32)tile_side_in_pixels*0.5f
	};

	for(int rel_row=-50; rel_row<50; ++rel_row)
	{
		for(int rel_col=-90; rel_col<90; ++rel_col)
		{
			uint32 col = game_state->player_position.tile_x + rel_col;
			uint32 row = game_state->player_position.tile_y + rel_row;

			uint32 tile_value = tiles::tile_value(tile_map, col, row, game_state->player_position.tile_z);
			if(tile_value)
			{
				float32 gray = 0.5f;
				if(tile_value == 2)
				{
					gray = 1.0f;
				}
				else if(tile_value == 3 || tile_value == 4)
				{
					gray = 0.2f;
				}

				if(row == game_state->player_position.tile_y && col == game_state->player_position.tile_x)
				{
					gray = 0.0f;
				}

				math::vector2 center_offset_tile_position = {
					screen_center.x + (float32)(rel_col*tile_side_in_pixels) - game_state->player_position.tile_relative.x*metres_to_pixels,
					screen_center.y - (float32)(rel_row*tile_side_in_pixels) + game_state->player_position.tile_relative.y*metres_to_pixels
				};
				math::vector2 min = center_offset_tile_position - tile_side_half_extents_in_pixels;
				math::vector2 max = center_offset_tile_position + tile_side_half_extents_in_pixels;
				_draw_rectangle(render_output, min, max, gray, gray, gray);
			}
		}
	}

	math::vector2 player_position = screen_center;

	_draw_bitmap(&render_output, &game_state->player_bitmap, player_position);

	// float32 player_min_x = player_x - metres_to_pixels*player_width*0.5f;
	// float32 player_max_x = player_x + metres_to_pixels*player_width*0.5f;
	// float32 player_min_y = player_y - metres_to_pixels*player_height;
	// float32 player_max_y = player_y;
	// _draw_rectangle(render_output, player_min_x, player_min_y, player_max_x, player_max_y, 0.9f, 0.1f, 0.2f);
}

extern "C" GAME_FILL_AUDIO_OUTPUT(fill_audio_output)
{
	game_state* game_state = (game::game_state*)memory->permanent_storage;
}
}//namespace game