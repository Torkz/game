#pragma once

namespace game
{
struct chunk
{
	uint32_t* tiles;
};

struct tile_map
{
	memory_space* memory_space;

	uint32_t chunk_dimension;
	uint32_t chunk_shift;
	uint32_t chunk_mask;

	uint32_t num_chunks_x;
	uint32_t num_chunks_y;
	uint32_t num_chunks_z;

	chunk* chunks;

	float tile_side_in_metres;
};

struct chunk_position
{
	uint32_t chunk_x;
	uint32_t chunk_y;
	uint32_t chunk_z;

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
	uint32_t tile_z;

	float tile_relative_x;
	float tile_relative_y;
};
} //namespace game