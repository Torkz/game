
namespace game
{
namespace tiles
{

internal inline
chunk_position _tile_chunk_position_from_tile_coordinates(tile_map* tile_map, uint32 tile_x, uint32 tile_y, uint32 tile_z)
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
uint32 _tile_value_from_tile_chunk_position(tile_map* tile_map, chunk_position chunk_position)
{
	uint32 result = 0;
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
bool position_is_valid(tile_map* tile_map, tile_map_position position)
{
	chunk_position chunk_position = _tile_chunk_position_from_world_position(tile_map, position);
	uint32 tile_value = _tile_value_from_tile_chunk_position(tile_map, chunk_position);

	if(
		tile_value == 1 ||
		tile_value == 3 ||
		tile_value == 4)
	{
		return true;
	}

	return false;
}

internal inline
uint32 tile_value(tile_map* tile_map, uint32 tile_x, uint32 tile_y, uint32 tile_z)
{
	uint32 result;
	chunk_position chunk_position = _tile_chunk_position_from_tile_coordinates(tile_map, tile_x, tile_y, tile_z);
	result = _tile_value_from_tile_chunk_position(tile_map, chunk_position);
	return result;
}

internal inline
uint32 tile_value(tile_map* tile_map, tile_map_position position)
{
	uint32 result = tile_value(tile_map, position.tile_x, position.tile_y, position.tile_z);
	return result;
}

internal inline
void set_tile_value(tile_map* tile_map, uint32 tile_x, uint32 tile_y, uint32 tile_z, uint32 tile_value)
{
	chunk_position chunk_position = _tile_chunk_position_from_tile_coordinates(tile_map, tile_x, tile_y, tile_z);
	chunk* chunk = _tile_chunk_from_tile_chunk_position(tile_map, chunk_position);

	if(!chunk->tiles)
	{
		chunk->tiles = _push_array(tile_map->memory_space, uint32, tile_map->chunk_dimension*tile_map->chunk_dimension);

		for(uint32 i = 0;
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
void _recanonicalize_coordinate(tile_map* tile_map, uint32* tile, float32* tile_relative)
{
	int32 tile_offset = math::round_float_to_int(*tile_relative / tile_map->tile_side_in_metres);
	*tile += tile_offset;
	*tile_relative -= tile_map->tile_side_in_metres*tile_offset;

	assert(*tile_relative >= -tile_map->tile_side_in_metres*0.5f);
	assert(*tile_relative < tile_map->tile_side_in_metres*0.5f);
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
	bool result = false;
	if(
		(a.tile_x == b.tile_x) &&
		(a.tile_y == b.tile_y) &&
		(a.tile_z == b.tile_z)
	)
	{
		result = true;
	}
	return result;
}

} //namespace tiles
} //namespace game