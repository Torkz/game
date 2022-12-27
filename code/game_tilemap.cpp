
namespace game
{
namespace tiles
{

internal inline
void _recanonicalize_coordinate(tile_map* tile_map, uint32_t* tile, float* tile_relative)
{
	int32_t tile_offset = math::floor_float_to_int(*tile_relative / tile_map->tile_side_in_metres);
	*tile += tile_offset;
	*tile_relative -= tile_map->tile_side_in_metres*tile_offset;

	assert(*tile_relative >= 0.0f);
	assert(*tile_relative < tile_map->tile_side_in_metres);
}

internal inline
tile_map_position recanonicalize_tile_map_position(tile_map* tile_map, tile_map_position position)
{
	tile_map_position result = position;

	_recanonicalize_coordinate(tile_map, &result.tile_x, &result.tile_relative_x);
	_recanonicalize_coordinate(tile_map, &result.tile_y, &result.tile_relative_y);

	return result;
}

internal inline
chunk_position _tile_chunk_position_from_world_position(tile_map* tile_map, tile_map_position position)
{
	chunk_position result;

	result.chunk_x = position.tile_x >> tile_map->chunk_shift;
	result.chunk_y = position.tile_y >> tile_map->chunk_shift;
	result.tile_x = position.tile_x & tile_map->chunk_mask;
	result.tile_y = position.tile_y & tile_map->chunk_mask;

	return result;
}

internal inline
chunk_position _tile_chunk_position_from_tile_coordinates(tile_map* tile_map, uint32_t tile_x, uint32_t tile_y)
{
	chunk_position result;

	result.chunk_x = tile_x >> tile_map->chunk_shift;
	result.chunk_y = tile_y >> tile_map->chunk_shift;
	result.tile_x = tile_x & tile_map->chunk_mask;
	result.tile_y = tile_y & tile_map->chunk_mask;

	return result;
}

internal inline chunk* _tile_chunk_from_tile_chunk_position(tile_map* tile_map, chunk_position chunk_position)
{
	return &tile_map->chunks[chunk_position.chunk_y*tile_map->chunk_dimension + chunk_position.chunk_x];
}

internal inline
uint32_t _tile_value_from_tile_chunk_position(tile_map* tile_map, chunk_position chunk_position)
{
	uint32_t result;
	chunk* chunk = _tile_chunk_from_tile_chunk_position(tile_map, chunk_position);
	result = chunk->tiles[chunk_position.tile_y*tile_map->chunk_dimension + chunk_position.tile_x];
	return result;
}

internal inline
bool position_is_valid(tile_map* tile_map, tile_map_position position)
{
	chunk_position chunk_position = _tile_chunk_position_from_world_position(tile_map, position);
	uint32_t tile_value = _tile_value_from_tile_chunk_position(tile_map, chunk_position);

	if(tile_value == 0)
	{
		return true;
	}

	return false;
}

internal inline
uint32_t tile_value(tile_map* tile_map, uint32_t tile_x, uint32_t tile_y)
{
	uint32_t result;
	chunk_position chunk_position = _tile_chunk_position_from_tile_coordinates(tile_map, tile_x, tile_y);
	result = _tile_value_from_tile_chunk_position(tile_map, chunk_position);
	return result;
}

} //namespace tiles
} //namespace game