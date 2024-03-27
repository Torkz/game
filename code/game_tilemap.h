#pragma once

namespace game
{
struct chunk
{
	u32* tiles;
};

struct tile_map
{
	memory_space* memory_space;

	u32 chunk_dimension;
	u32 chunk_shift;
	u32 chunk_mask;

	u32 num_chunks_x;
	u32 num_chunks_y;
	u32 num_chunks_z;

	chunk* chunks;

	f32 tile_side_in_metres;
};

struct chunk_position
{
	u32 chunk_x;
	u32 chunk_y;
	u32 chunk_z;

	u32 tile_x;
	u32 tile_y;
};

struct tile_map_position
{
	//note(staffan): tile locations
	//high bits equals tile chunk
	//low bits equals tile index in the chunk.
	u32 tile_x;
	u32 tile_y;
	u32 tile_z;

	math::v2 tile_relative;
};

struct tile_map_position_difference
{
	math::v2 xy;
	i32 z;
};

} //namespace game