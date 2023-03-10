#pragma once

namespace game
{
struct chunk
{
	uint32* tiles;
};

struct tile_map
{
	memory_space* memory_space;

	uint32 chunk_dimension;
	uint32 chunk_shift;
	uint32 chunk_mask;

	uint32 num_chunks_x;
	uint32 num_chunks_y;
	uint32 num_chunks_z;

	chunk* chunks;

	float32 tile_side_in_metres;
};

struct chunk_position
{
	uint32 chunk_x;
	uint32 chunk_y;
	uint32 chunk_z;

	uint32 tile_x;
	uint32 tile_y;
};

struct tile_map_position
{
	//note(staffan): tile locations
	//high bits equals tile chunk
	//low bits equals tile index in the chunk.
	uint32 tile_x;
	uint32 tile_y;
	uint32 tile_z;

#if 0
	float32 tile_relative_x;
	float32 tile_relative_y;
#else
	math::vector2 tile_relative;
#endif
};
} //namespace game