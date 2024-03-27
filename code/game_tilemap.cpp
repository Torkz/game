
namespace game
{
namespace tiles
{

internal inline
chunk_position _tile_chunk_position_from_tile_coordinates(tile_map* tile_map, u32 tile_x, u32 tile_y, u32 tile_z)
{
	chunk_position result;

	result.chunk_x = tile_x >> tile_map->chunk_shift;
	result.chunk_y = tile_y >> tile_map->chunk_shift;
	result.chunk_z = tile_z;
	result.tile_x = tile_x & tile_map->chunk_mask;
	result.tile_y = tile_y & tile_map->chunk_mask;

	return result;
}

internal inline
chunk_position _tile_chunk_position_from_world_position(tile_map* tile_map, tile_map_position position)
{
	chunk_position result = _tile_chunk_position_from_tile_coordinates(tile_map, position.tile_x, position.tile_y, position.tile_z);
	return result;
}

internal inline chunk* _tile_chunk_from_tile_chunk_position(tile_map* tile_map, chunk_position chunk_position)
{
	return &tile_map->chunks[
		chunk_position.chunk_z*tile_map->num_chunks_y*tile_map->num_chunks_x +
		chunk_position.chunk_y*tile_map->num_chunks_x +
		chunk_position.chunk_x];
}

internal inline
u32 _tile_value_from_tile_chunk_position(tile_map* tile_map, chunk_position chunk_position)
{
	u32 result = 0;
	if(
		chunk_position.chunk_x < tile_map->num_chunks_x &&
		chunk_position.chunk_y < tile_map->num_chunks_y &&
		chunk_position.chunk_z < tile_map->num_chunks_z
	)
	{
		chunk* chunk = _tile_chunk_from_tile_chunk_position(tile_map, chunk_position);
		if(chunk->tiles)
		{
			result = chunk->tiles[chunk_position.tile_y*tile_map->chunk_dimension + chunk_position.tile_x];
		}
	}
	return result;
}

internal inline
bool tile_value_is_valid(u32 tile_value)
{
	bool result;
	if(
		tile_value == 1 ||
		tile_value == 3 ||
		tile_value == 4)
	{
		result = true;
	}
	else
	{
		result = false;
	}
	return result;
}

internal inline
bool position_is_valid(tile_map* tile_map, tile_map_position position)
{
	chunk_position chunk_position = _tile_chunk_position_from_world_position(tile_map, position);
	u32 tile_value = _tile_value_from_tile_chunk_position(tile_map, chunk_position);
	return tile_value_is_valid(tile_value);
}

internal inline
bool position_is_valid(tile_map* tile_map, u32 tile_x, u32 tile_y, u32 tile_z)
{
	chunk_position chunk_position = _tile_chunk_position_from_tile_coordinates(tile_map, tile_x, tile_y, tile_z);
	u32 tile_value = _tile_value_from_tile_chunk_position(tile_map, chunk_position);
	return tile_value_is_valid(tile_value);
}

internal inline
u32 tile_value(tile_map* tile_map, u32 tile_x, u32 tile_y, u32 tile_z)
{
	u32 result;
	chunk_position chunk_position = _tile_chunk_position_from_tile_coordinates(tile_map, tile_x, tile_y, tile_z);
	result = _tile_value_from_tile_chunk_position(tile_map, chunk_position);
	return result;
}

internal inline
u32 tile_value(tile_map* tile_map, tile_map_position position)
{
	u32 result = tile_value(tile_map, position.tile_x, position.tile_y, position.tile_z);
	return result;
}

internal inline
void set_tile_value(tile_map* tile_map, u32 tile_x, u32 tile_y, u32 tile_z, u32 tile_value)
{
	chunk_position chunk_position = _tile_chunk_position_from_tile_coordinates(tile_map, tile_x, tile_y, tile_z);
	chunk* chunk = _tile_chunk_from_tile_chunk_position(tile_map, chunk_position);

	if(!chunk->tiles)
	{
		chunk->tiles = _push_array(tile_map->memory_space, u32, tile_map->chunk_dimension*tile_map->chunk_dimension);

		for(u32 i = 0;
			i < tile_map->chunk_dimension*tile_map->chunk_dimension;
			++i)
		{
			chunk->tiles[i] = 1;
		}
	}

	chunk->tiles[chunk_position.tile_y*tile_map->chunk_dimension + chunk_position.tile_x] = tile_value;
}

// todo(staffan): figure it out man.
// tile_map_position stuff might be better placed somewhere else than tiles namespace.
internal inline
void _recanonicalize_coordinate(tile_map* tile_map, u32* tile, f32* tile_relative)
{
	#if GAME_SLOWMODE
	u32 tile_prev = *tile;
	f32 tile_rel_prev = *tile_relative;
	#endif

	i32 tile_offset = math::round_f32_to_i32(*tile_relative / tile_map->tile_side_in_metres);
	*tile += tile_offset;
	*tile_relative -= tile_map->tile_side_in_metres*tile_offset;

	assert(*tile_relative >= -tile_map->tile_side_in_metres*0.5f);
	assert(*tile_relative <= tile_map->tile_side_in_metres*0.5f);
}

internal inline
tile_map_position recanonicalize_tile_map_position(tile_map* tile_map, tile_map_position position)
{
	tile_map_position result = position;

	_recanonicalize_coordinate(tile_map, &result.tile_x, &result.tile_relative.x);
	_recanonicalize_coordinate(tile_map, &result.tile_y, &result.tile_relative.y);

	return result;
}

internal inline
bool are_on_same_tile(tile_map_position a, tile_map_position b)
{
	bool result;
	if(
		(a.tile_x == b.tile_x) &&
		(a.tile_y == b.tile_y) &&
		(a.tile_z == b.tile_z)
	)
	{
		result = true;
	}
	else
	{
		result = false;
	}
	return result;
}

internal inline
tile_map_position_difference difference(tile_map* tile_map, tile_map_position a, tile_map_position b)
{
	tile_map_position_difference result;
	result.xy.x = ((f32)b.tile_x*tile_map->tile_side_in_metres + b.tile_relative.x)
					-((f32)a.tile_x*tile_map->tile_side_in_metres + a.tile_relative.x);
	result.xy.y = ((f32)b.tile_y*tile_map->tile_side_in_metres + b.tile_relative.y)
					-((f32)a.tile_y*tile_map->tile_side_in_metres + a.tile_relative.y);
	result.z = b.tile_z-a.tile_z;
	return result;
}

struct tile_edge
{
	math::v2 closest_position;
	math::v2 normal;
};
internal inline
tile_edge closest_tile_edge(tile_map_position tile_position, tile_map_position compare_position)
{
	tile_edge result;
	

	

	return result;
}

} //namespace tiles
} //namespace game