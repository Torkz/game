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

extern "C" GAME_UPDATE_AND_RENDER(update_and_render)
{
	assert(memory->permanent_storage_size >= sizeof(game::game_state));
	game_state* game_state = (game::game_state*)memory->permanent_storage;
	if(!memory->is_initialized)
	{
		game_state->player_position.tile_x = 3;
		game_state->player_position.tile_y = 3;
		game_state->player_position.tile_relative_x = 4.5f;
		game_state->player_position.tile_relative_y = 4.5f;

		memory->is_initialized = true;
	}

	input::new_frame(&input);

#define TILE_MAP_X 256
#define TILE_MAP_Y 256
	uint32_t temp_tiles[TILE_MAP_Y][TILE_MAP_X] =
	{
		{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,	 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,	 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,	 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
		{1, 0, 0, 1,  0, 0, 1, 1,  1, 0, 1, 1,  1, 0, 0, 1,	 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
		{1, 0, 0, 1,  0, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 0,	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
		{1, 0, 0, 1,  0, 0, 0, 0,  0, 0, 1, 0,  1, 0, 0, 1,	 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
		{1, 0, 0, 0,  1, 0, 0, 0,  0, 0, 1, 1,  1, 0, 0, 1,	 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
		{1, 0, 0, 0,  1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,	 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
		{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,	 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
		{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,	 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,	 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,	 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,	 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,	 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,	 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,	 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,	 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
		{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,	 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1}
	};

	tile_map tile_map;
	tile_map.chunk_shift = 8;
	tile_map.chunk_dimension = (1 << tile_map.chunk_shift);
	tile_map.chunk_mask = tile_map.chunk_dimension - 1;

	tile_map.tile_side_in_metres = 2.0f;
	tile_map.tile_side_in_pixels = 80;
	tile_map.metres_to_pixels = (float)tile_map.tile_side_in_pixels/tile_map.tile_side_in_metres;

	chunk tile_chunk;
	tile_chunk.tiles = (uint32_t*)temp_tiles;
	tile_map.chunks = &tile_chunk;
	
	world world = {};
	world.tile_map = &tile_map;
	game_state->world = &world;

	float player_height = 1.8f;
	float player_width = player_height*0.75f;

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
	float speed_this_frame = speed*time.dt;
	delta_x *= speed_this_frame;
	delta_y *= speed_this_frame;

	tile_map_position new_position = game_state->player_position;
	new_position.tile_relative_x += delta_x;
	new_position.tile_relative_y += delta_y;
	new_position = tiles::recanonicalize_tile_map_position(world.tile_map, new_position);

	tile_map_position left_position = new_position;
	left_position.tile_relative_x -= player_width*0.5f;
	left_position = tiles::recanonicalize_tile_map_position(world.tile_map, left_position);

	tile_map_position right_position = new_position;
	right_position.tile_relative_x += player_width*0.5f;
	right_position = tiles::recanonicalize_tile_map_position(world.tile_map, right_position);

	bool position_is_valid = tiles::position_is_valid(world.tile_map, new_position)
							&& tiles::position_is_valid(world.tile_map, left_position)
							&& tiles::position_is_valid(world.tile_map, right_position);

	if(position_is_valid)
	{
		game_state->player_position = new_position;
	}

	_draw_rectangle(render_output, 0.0f, 0.0f, (float)render_output.width, (float)render_output.height, 1.0f, 0.0f, 1.0f);

	float center_x = render_output.width*0.5f;
	float center_y = render_output.height*0.5f;

	for(int rel_row=-10; rel_row<10; ++rel_row)
	{
		for(int rel_col=-20; rel_col<20; ++rel_col)
		{
			uint32_t col = game_state->player_position.tile_x + rel_col;
			uint32_t row = game_state->player_position.tile_y + rel_row;

			if(rel_col < 0 && col > game_state->player_position.tile_x)
			{
				col = 0;
			}
			if(rel_row < 0 && row > game_state->player_position.tile_y)
			{
				row = 0;
			}

			float r = 0.5f;
			float g = 0.5f;
			float b = 0.5f;
			if(tiles::tile_value(world.tile_map, col, row) == 1)
			{
				r = 1.0f;
				g = 1.0f;
				b = 1.0f;
			}

			if(row == game_state->player_position.tile_y && col == game_state->player_position.tile_x)
			{
				r = 0.0f;
				g = 0.0f;
				b = 0.0f;
			}

			float min_x = center_x + (float)(rel_col*tile_map.tile_side_in_pixels) - game_state->player_position.tile_relative_x*tile_map.metres_to_pixels;
			float min_y = center_y - (float)(rel_row*tile_map.tile_side_in_pixels) + game_state->player_position.tile_relative_y*tile_map.metres_to_pixels;
			float max_x = min_x + (float)tile_map.tile_side_in_pixels;
			float max_y = min_y - (float)tile_map.tile_side_in_pixels;
			_draw_rectangle(render_output, min_x, max_y, max_x, min_y, r, g, b);
		}
	}

	float player_x = center_x;
	float player_y = center_y;

	float player_min_x = player_x - tile_map.metres_to_pixels*player_width*0.5f;
	float player_max_x = player_x + tile_map.metres_to_pixels*player_width*0.5f;
	float player_min_y = player_y - tile_map.metres_to_pixels*player_height;
	float player_max_y = player_y;
	_draw_rectangle(render_output, player_min_x, player_min_y, player_max_x, player_max_y, 0.9f, 0.1f, 0.2f);
}

extern "C" GAME_FILL_AUDIO_OUTPUT(fill_audio_output)
{
	game_state* game_state = (game::game_state*)memory->permanent_storage;
}
}//namespace game