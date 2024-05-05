#include "game_tilemap.cpp"

namespace game
{
	#pragma pack(push, 1)
	struct bmp_header
	{
		u16 file_type;     /* File type, always 4D42h ("BM") */
		u32 file_size;     /* Size of the file in bytes */
		u16 reserved_1;    /* Always 0 */
		u16 reserved_2;    /* Always 0 */
		u32 bitmap_offset; /* Starting position of image data in bytes */
		u32 header_size;   /* Size of this header in bytes */
		i32 width;           /* Image width in pixels */
		i32 height;          /* Image height in pixels */
		u16 planes;          /* Number of color planes */
		u16 bits_per_pixel;    /* Number of bits per pixel */
		u32 compression;     /* Compression methods used */
		u32 size_of_bitmap;    /* Size of bitmap in bytes */
		i32 horzizontal_resolution;  /* Horizontal resolution in pixels per meter */
		i32 vertical_resolution;  /* Vertical resolution in pixels per meter */
		u32 colors_used;      /* Number of colors in the image */
		u32 colors_important; /* Minimum number of important colors */

		u32 red_mask;         /* Mask identifying bits of red component */
		u32 green_mask;       /* Mask identifying bits of green component */
		u32 blue_mask;        /* Mask identifying bits of blue component */
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
		result.pixels = (u32*)((u8*)read_result.content + header->bitmap_offset);
		result.width = header->width;
		result.height = header->height;

		assert(header->compression == 3);

		u32 red_mask = header->red_mask;
		u32 green_mask = header->green_mask;
		u32 blue_mask = header->blue_mask;
		u32 alpha_mask = ~(red_mask | green_mask | blue_mask);

		using namespace bit;
		scan_bit_result red_shift = find_least_significant_set_bit(red_mask);
		scan_bit_result green_shift = find_least_significant_set_bit(green_mask);
		scan_bit_result blue_shift = find_least_significant_set_bit(blue_mask);
		scan_bit_result alpha_shift = find_least_significant_set_bit(alpha_mask);
		assert(red_shift.found && green_shift.found && blue_shift.found && alpha_shift.found);

		u32* source_dest = result.pixels;
		for(i32 i=0;
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

	internal void _draw_rectangle(render_output* output, math::v2 min, math::v2 max, f32 red, f32 green, f32 blue)
	{
		i32 min_x_rounded = math::round_f32_to_u32(min.x);
		i32 min_y_rounded = math::round_f32_to_u32(min.y);
		i32 max_x_rounded = math::round_f32_to_u32(max.x);
		i32 max_y_rounded = math::round_f32_to_u32(max.y);

		if(min_x_rounded < 0)
		{
			min_x_rounded = 0;
		}

		if(min_y_rounded < 0)
		{
			min_y_rounded = 0;
		}

		if(max_x_rounded > output->width)
		{
			max_x_rounded = output->width;
		}

		if(max_y_rounded > output->height)
		{
			max_y_rounded = output->height;
		}

		u32 color =
			(math::round_f32_to_u32(red*255.0f)<<16) |
			(math::round_f32_to_u32(green*255.0f)<<8) |
			math::round_f32_to_u32(blue*255.0f);

		u8* row = (u8*)output->memory+(min_y_rounded*output->pitch);
		for(i32 y=min_y_rounded; y<max_y_rounded; ++y)
		{
			for(i32 x=min_x_rounded; x<max_x_rounded; ++x)
			{		
				*((u32*)row+x) = color;
			}
			row += output->pitch;
		}
	}

	internal void
	_draw_bitmap(render_output* output, loaded_bitmap* bitmap, math::v2 start)
	{
		i32 blit_width = bitmap->width;
		i32 blit_height = bitmap->height;

		i32 rounded_start_x = math::round_f32_to_u32(start.x) + bitmap->offset_x;
		i32 rounded_start_y = math::round_f32_to_u32(start.y) + bitmap->offset_y;

		u32* source_row = bitmap->pixels + bitmap->width*(bitmap->height-1);

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

		u8* dest_row = (u8*)((u32*)((u8*)output->memory + rounded_start_y*output->pitch) + rounded_start_x);
		for(i32 y=0;
			y<blit_height;
			++y)
		{
			for(i32 x=0;
				x<blit_width;
				++x)
			{
				u32* source = source_row+x;
				u32* dest = (u32*)dest_row+x;

				if(*source & 0xFF000000)
				{
					*dest = *source;
				}
			}
			source_row -= bitmap->width;
			dest_row += output->pitch;
		}
	}

	internal void _initialize_2d_exploration(thread_context* thread, game_state* game_state, game_memory* memory, memory_index memory_start_location)
	{
		game_state->test_bitmap = _debug_load_bmp(thread, memory->debug_read_entire_file, "test/test_background.bmp");
		game_state->test_bitmap.offset_x = 0;
		game_state->test_bitmap.offset_y = 0;
		game_state->player_bitmap = _debug_load_bmp(thread, memory->debug_read_entire_file, "test/character01.bmp");
		game_state->player_bitmap.offset_x = -32;
		game_state->player_bitmap.offset_y = -59;

		game_state->player_position.tile_x = 3;
		game_state->player_position.tile_y = 3;

		_initialize_memory_space(&game_state->world_space,
			(u8*)memory->permanent_storage+memory_start_location,
			memory->permanent_storage_size-memory_start_location
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

		u32 num_screen_tiles_x = 17;
		u32 num_screen_tiles_y = 9;
		u32 screen_x = 0;
		u32 screen_y = 0;
		u32 screen_z = 0;
		bool door_top = false;
		bool door_right = false;
		bool door_bottom = false;
		bool door_left = false;
		bool door_up = false;
		bool door_down = false;
		bool created_z_door = false;

		for(u32 screen_index = 0;
			screen_index < 100;
			++screen_index)
		{
			if(screen_index != 99)
			{
				u32 random = random_number(screen_index);

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
					u32 door_case = random%3;
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

			for(u32 y = 0;
				y < num_screen_tiles_y;
				++y)
			{
				for(u32 x = 0;
					x < num_screen_tiles_x;
					++x)
				{
					u32 tile_x = (screen_x*num_screen_tiles_x) + x;
					u32 tile_y = (screen_y*num_screen_tiles_y) + y;

					u32 tile_value = 1;
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
    }

    internal void _update_2d_exploration(game_state* game_state, game_time& time, render_output& render_output)
    {
		world* world = game_state->world;
		tile_map* tile_map = world->tile_map;

		i32 tile_side_in_pixels = 60;
		f32 metres_to_pixels = tile_side_in_pixels/tile_map->tile_side_in_metres;
			
		math::v2 delta = {};
		math::v2 input_direction = {};

		//note(staffan): not really friction, just used when there's no input in that direction.
		math::v2 friction_direction;
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

		f32 max_speed = 7.0f;
		math::v2 acceleration = input_direction*3.0f;
		math::v2 friction = friction_direction*1.0f;
		math::v2 wanted_velocity = game_state->player_velocity+friction;

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

		f32 speed_scale = input::button_held(SPACEBAR) ? 5.0f : 1.0f;
		math::v2 delta_this_frame = wanted_velocity*time.dt*speed_scale;

		math::v2 movement = delta_this_frame;
		math::v2 initial_direction = normalize(movement);
		math::v2 current_direction = initial_direction;
		f32 movement_length = length(movement);
		bool found_any_collision = false;

		tile_map_position new_position = game_state->player_position;
		math::v2 actual_velocity = wanted_velocity;

		f32 player_height = 1.8f;
		f32 player_width = player_height*0.75f;
		f32 player_collision_height = player_height*0.1f;

		i32 iterations = 0;
		while(movement_length > 0.0001f && dot(current_direction, initial_direction) > 0.0f)
		{
			iterations++;

			math::v2 player_half_extents = {player_width*0.5f, player_collision_height*0.5f};
			tile_map_position player_collision_box_pos = new_position;
			player_collision_box_pos.tile_relative.y += player_half_extents.y;
			player_collision_box_pos = tiles::recanonicalize_tile_map_position(tile_map, player_collision_box_pos);

			math::v2 collision_shape_half_extents = {tile_map->tile_side_in_metres*0.5f, tile_map->tile_side_in_metres*0.5f};
			math::shape collision_shapes[8];
			u32 num_collision_shapes = 0;
			for(i32 y = -1; y<2; ++y)
			{
				for(i32 x = -1; x<2; ++x)
				{
					if(x == 0 && y==0)
					{
						continue;
					}

					tile_map_position tile_pos;
					tile_pos.tile_x = new_position.tile_x+x;
					tile_pos.tile_y = new_position.tile_y+y;
					tile_pos.tile_z = new_position.tile_z;
					tile_pos.tile_relative = {0.0f, 0.0f};
					if(tiles::position_is_valid(tile_map, tile_pos) == false)
					{
						tile_map_position_difference diff = tiles::difference(tile_map, player_collision_box_pos, tile_pos);
						collision_shapes[num_collision_shapes++] = aabb_shape(diff.xy, collision_shape_half_extents);
					}
				}
			}

			math::v2 sweep_from = {0.0f, 0.0f};
			math::aabb_linear_sweep_result sr;
			sr = aabb_linear_sweep(sweep_from, movement, player_half_extents, collision_shapes, num_collision_shapes);

			if(sr.num_sweep_results > 0)
			{
				math::sweep_result closest_collision = sr.sweep_results[0];
				f32 free_distance = math::max(closest_collision.distance_on_sweep_vector-0.001f, 0.0f); //skin?
				math::v2 free_movement = current_direction * free_distance;
				new_position.tile_relative += free_movement;
				new_position = tiles::recanonicalize_tile_map_position(tile_map, new_position);

				f32 dot = math::dot(movement, closest_collision.normal);
				math::v2 reflection_vector = closest_collision.normal*dot;

				movement -= reflection_vector;
				current_direction = normalize(movement);

				movement_length = math::max(length(movement) - free_distance, 0.0f);
				actual_velocity = movement/time.dt; //todo(staffan): this might be wrong to do here.
			}
			else
			{
				new_position.tile_relative += movement;
				new_position = tiles::recanonicalize_tile_map_position(tile_map, new_position);
				break;
			}
		}

		if(!tiles::are_on_same_tile(game_state->player_position, new_position))
		{
			u32 tile_value = tiles::tile_value(tile_map, new_position);
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
		game_state->player_velocity = actual_velocity;

		if(input::button_held(MOUSE_LEFT))
		{
			game_state->player_position.tile_x = 3;
			game_state->player_position.tile_y = 3;
			game_state->player_position.tile_z = 0;
			game_state->player_position.tile_relative = {0.0f, 0.0f};
			game_state->player_velocity = {0.0f, 0.0f};
		}
		_draw_rectangle(&render_output, {0.0f, 0.0f}, {(f32)render_output.width, (f32)render_output.height}, 0.2f, 0.2f, 0.2f);
		// _draw_bitmap(&render_output, &game_state->test_bitmap, {0.0f, 0.0f});

		math::v2 screen_center = {
			render_output.width*0.5f,
			render_output.height*0.5f
		};
		math::v2 tile_side_half_extents_in_pixels = {
			(f32)tile_side_in_pixels*0.5f,
			(f32)tile_side_in_pixels*0.5f
		};

		for(i32 rel_row=-50; rel_row<50; ++rel_row)
		{
			for(i32 rel_col=-90; rel_col<90; ++rel_col)
			{
				u32 col = game_state->player_position.tile_x + rel_col;
				u32 row = game_state->player_position.tile_y + rel_row;

				u32 tile_value = tiles::tile_value(tile_map, col, row, game_state->player_position.tile_z);
				if(tile_value)
				{
					f32 gray = 0.5f;
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

					math::v2 center_offset_tile_position = {
						screen_center.x + (f32)(rel_col*tile_side_in_pixels) - game_state->player_position.tile_relative.x*metres_to_pixels,
						screen_center.y - (f32)(rel_row*tile_side_in_pixels) + game_state->player_position.tile_relative.y*metres_to_pixels
					};
					math::v2 min = center_offset_tile_position - tile_side_half_extents_in_pixels;
					math::v2 max = center_offset_tile_position + tile_side_half_extents_in_pixels;
					// _draw_rectangle(&render_output, min, max, gray, gray, gray);
				}
			}
		}

		math::v2 player_position = screen_center;
		// _draw_bitmap(&render_output, &game_state->player_bitmap, player_position);

    }
}