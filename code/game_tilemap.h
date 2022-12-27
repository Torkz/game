#pragma once

namespace game
{
struct chunk
{
	uint32_t* tiles;
};

struct tile_map
{
	uint32_t chunk_dimension;
	uint32_t chunk_shift;
	uint32_t chunk_mask;

	int tile_side_in_pixels;
	float tile_side_in_metres;
	float metres_to_pixels;

	chunk* chunks;
};

struct chunk_position
{
	uint32_t chunk_x;
	uint32_t chunk_y;

	uint32_t tile_x;
	uint32_t tile_y;
};

struct tile_map_position
{
	//note(staffan): tile locations
	//high bits equals tile chunk
	//low bits equals tile index in the chunk.
	uint32_t tile_x;
	uint32_t tile_y;

	float tile_relative_x;
	float tile_relative_y;
};
} //namespace game