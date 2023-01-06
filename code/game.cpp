#include "game.h"
#include "game_math.cpp"
#include "game_input.cpp"
#include "game_tilemap.cpp"

namespace game
{

#if 0
internal void _draw_rectangle(render_output& output, float min_x, float min_y, float max_x, float max_y, float red, float green, float blue)
{
	int min_x_trunc = (int)min_x;
	int min_y_trunc = (int)min_y;
	int max_x_ceiled = _ceil_float_to_int(max_x);
	int max_y_ceiled = _ceil_float_to_int(max_y);

	if(min_x_trunc < 0)
	{
		min_x_trunc = 0;
	}

	if(min_y_trunc < 0)
	{
		min_y_trunc = 0;
	}

	if(max_x_ceiled > output.width)
	{
		max_x_ceiled = output.width;
	}

	if(max_y_ceiled > output.height)
	{
		max_y_ceiled = output.height;
	}

	float r = red*255.0f;
	float g = green*255.0f;
	float b = blue*255.0f;

	float max_x_alpha = min_x - (float)min_x_trunc;
	float min_x_alpha = 1.0f - max_x_alpha;
	float max_y_alpha = min_y - (float)min_y_trunc;
	float min_y_alpha = 1.0f - max_y_alpha;

	uint8_t* row = (uint8_t*)output.memory+(min_y_trunc*output.pitch);
	for(int y=min_y_trunc; y<max_y_ceiled; ++y)
	{
		float y_alpha = 1.0f;
		y_alpha = y==min_y_trunc ? min_y_alpha : y_alpha;
		y_alpha = y==(max_y_ceiled-1) ? max_y_alpha : y_alpha;

		for(int x=min_x_trunc; x<max_x_ceiled; ++x)
		{
			float x_alpha = 1.0f;
			x_alpha = x==min_x_trunc ? min_x_alpha : x_alpha;
			x_alpha = x==(max_x_ceiled-1) ? max_x_alpha : x_alpha;

			float alpha = x_alpha < y_alpha ? x_alpha : y_alpha;
			
			uint32_t* pixel = (uint32_t*)row+x;
			uint8_t pixel_r = ((*pixel >> 16) & 0xff);
			uint8_t pixel_g = ((*pixel >> 8) & 0xff);
			uint8_t pixel_b = (*pixel & 0xff);

			*((uint32_t*)row+x) =
				(_round_float_to_uint32(_lerp((float)pixel_r, r, alpha))<<16) +
				(_round_float_to_uint32(_lerp((float)pixel_g, g, alpha))<<8) +
				_round_float_to_uint32(_lerp((float)pixel_b, b, alpha));
		}
		row += output.pitch;
	}
}
#else
internal void _draw_rectangle(render_output& output, float min_x, float min_y, float max_x, float max_y, float red, float green, float blue)
{
	int min_x_rounded = math::round_float_to_uint32(min_x);
	int min_y_rounded = math::round_float_to_uint32(min_y);
	int max_x_rounded = math::round_float_to_uint32(max_x);
	int max_y_rounded = math::round_float_to_uint32(max_y);

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

	uint32_t color =
		(math::round_float_to_uint32(red*255.0f)<<16) +
		(math::round_float_to_uint32(green*255.0f)<<8) +
		math::round_float_to_uint32(blue*255.0f);

	uint8_t* row = (uint8_t*)output.memory+(min_y_rounded*output.pitch);
	for(int y=min_y_rounded; y<max_y_rounded; ++y)
	{
		for(int x=min_x_rounded; x<max_x_rounded; ++x)
		{		
			*((uint32_t*)row+x) = color;
		}
		row += output.pitch;
	}
}
#endif

internal
void _initialize_memory_space(memory_space* space, uint8_t* base, memory_index max_memory)
{
	space->base = base;
	space->max_memory = max_memory;
	space->used_memory = 0;
}



extern "C" GAME_UPDATE_AND_RENDER(update_and_render)
{
	assert(memory->permanent_storage_size >= sizeof(game::game_state));
	game_state* game_state = (game::game_state*)memory->permanent_storage;

	float player_height = 1.8f;
	float player_width = player_height*0.75f;

	if(!memory->is_initialized)
	{
		game_state->player_position.tile_x = 3;
		game_state->player_position.tile_y = 3;
		game_state->player_position.tile_z = 0;
		game_state->player_position.tile_relative_x = 0.0f;
		game_state->player_position.tile_relative_y = 0.0f;

		_initialize_memory_space(&game_state->world_space,
			(uint8_t*)memory->permanent_storage+sizeof(game::game_state),
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

		uint32_t num_screen_tiles_x = 17;
		uint32_t num_screen_tiles_y = 9;
		uint32_t screen_x = 0;
		uint32_t screen_y = 0;
		uint32_t screen_z = 0;
		bool door_top = false;
		bool door_right = false;
		bool door_bottom = false;
		bool door_left = false;
		bool door_up = false;
		bool door_down = false;
		bool created_z_door = false;

		for(uint32_t screen_index = 0;
			screen_index < 100;
			++screen_index)
		{
			if(screen_index != 99)
			{
				uint32_t random = random_number(screen_index);

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
					uint32_t door_case = random%3;
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

			for(uint32_t y = 0;
				y < num_screen_tiles_y;
				++y)
			{
				for(uint32_t x = 0;
					x < num_screen_tiles_x;
					++x)
				{
					uint32_t tile_x = (screen_x*num_screen_tiles_x) + x;
					uint32_t tile_y = (screen_y*num_screen_tiles_y) + y;

					uint32_t tile_value = 1;
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

	int tile_side_in_pixels = 8;
	float metres_to_pixels = tile_side_in_pixels/tile_map->tile_side_in_metres;

	input::new_frame(&input);

	float delta_x = 0.0f;
	float delta_y = 0.0f;

	if(input::button_held(A))
	{
		delta_x += -1.0f;
	}
	if(input::button_held(D))
	{
		delta_x += 1.0f;
	}
	if(input::button_held(W))
	{
		delta_y += 1.0f;
	}
	if(input::button_held(S))
	{
		delta_y += -1.0f;
	}

	float speed = 6.0f;
	if(input::button_held(SPACEBAR))
	{
		speed = 20.0f;
	}
	float speed_this_frame = speed*time.dt;
	delta_x *= speed_this_frame;
	delta_y *= speed_this_frame;

	tile_map_position new_position = game_state->player_position;
	new_position.tile_relative_x += delta_x;
	new_position.tile_relative_y += delta_y;
	new_position = tiles::recanonicalize_tile_map_position(tile_map, new_position);

	tile_map_position left_position = new_position;
	left_position.tile_relative_x -= player_width*0.5f;
	left_position = tiles::recanonicalize_tile_map_position(tile_map, left_position);

	tile_map_position right_position = new_position;
	right_position.tile_relative_x += player_width*0.5f;
	right_position = tiles::recanonicalize_tile_map_position(tile_map, right_position);

	bool position_is_valid = tiles::position_is_valid(tile_map, new_position)
							&& tiles::position_is_valid(tile_map, left_position)
							&& tiles::position_is_valid(tile_map, right_position);

	if(position_is_valid)
	{
		if(!tiles::are_on_same_tile(game_state->player_position, new_position))
		{
			uint32_t tile_value = tiles::tile_value(tile_map, new_position);
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
	}

	_draw_rectangle(render_output, 0.0f, 0.0f, (float)render_output.width, (float)render_output.height, 1.0f, 0.0f, 1.0f);

	float screen_center_x = render_output.width*0.5f;
	float screen_center_y = render_output.height*0.5f;

	for(int rel_row=-50; rel_row<50; ++rel_row)
	{
		for(int rel_col=-90; rel_col<90; ++rel_col)
		{
			uint32_t col = game_state->player_position.tile_x + rel_col;
			uint32_t row = game_state->player_position.tile_y + rel_row;

			uint32_t tile_value = tiles::tile_value(tile_map, col, row, game_state->player_position.tile_z);
			if(tile_value)
			{
				float gray = 0.5f;
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

				float center_x = screen_center_x + (float)(rel_col*tile_side_in_pixels) - game_state->player_position.tile_relative_x*metres_to_pixels;
				float center_y = screen_center_y - (float)(rel_row*tile_side_in_pixels) + game_state->player_position.tile_relative_y*metres_to_pixels;
				float min_x = center_x - (float)tile_side_in_pixels*0.5f;
				float min_y = center_y - (float)tile_side_in_pixels*0.5f;
				float max_x = center_x + (float)tile_side_in_pixels*0.5f;
				float max_y = center_y + (float)tile_side_in_pixels*0.5f;
				_draw_rectangle(render_output, min_x, min_y, max_x, max_y, gray, gray, gray);
			}
		}
	}

	float player_x = screen_center_x;
	float player_y = screen_center_y;

	float player_min_x = player_x - metres_to_pixels*player_width*0.5f;
	float player_max_x = player_x + metres_to_pixels*player_width*0.5f;
	float player_min_y = player_y - metres_to_pixels*player_height;
	float player_max_y = player_y;
	_draw_rectangle(render_output, player_min_x, player_min_y, player_max_x, player_max_y, 0.9f, 0.1f, 0.2f);
}

extern "C" GAME_FILL_AUDIO_OUTPUT(fill_audio_output)
{
	game_state* game_state = (game::game_state*)memory->permanent_storage;
}
}//namespace game