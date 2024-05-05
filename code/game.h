//
//	Compilers
//
#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif

#if !defined(COMPILER_LLVM)
#define COMPILER_LLVM 0
#endif

#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif

#if COMPILER_MSVC
#include <intrin.h>
#pragma intrinsic(_BitScanForward)
#pragma intrinsic(__debugbreak)
#endif
//
//	/Compilers
//

typedef signed char        	i8;
typedef short              	i16;
typedef int                	i32;
typedef long long          	i64;
typedef unsigned char      	u8;
typedef unsigned short     	u16;
typedef unsigned int       	u32;
typedef unsigned long long 	u64;
typedef float				f32;
typedef double				f64;

#define global_variable static
#define internal static
#define local_persist static
#define const_exp static

#define PI 3.14159265359f
#define TAU PI*2.0f

#define memory_index size_t

#define kilobytes(value) ((value)*1024)
#define megabytes(value) (kilobytes(value)*1024)
#define gigabytes(value) (megabytes(value)*1024)
#define terabytes(value) (gigabytes(value)*1024)

// #define array_size(array, type) sizeof(array)/sizeof(type);

template <typename T>
internal inline
u32 array_size(T array)
{
	return sizeof(array)/sizeof(T);
}

#include "game_intrinsics.h"

namespace game
{
struct thread_context
{
	i32 placeholder;
};

#if GAME_INTERNAL
struct debug_read_file_results
{
	void* content;
	memory_index size;
};
#define DEBUG_READ_ENTIRE_FILE(name) game::debug_read_file_results name(game::thread_context* thread, char* file_name)
typedef DEBUG_READ_ENTIRE_FILE(debug_read_entire_file_def);

#define DEBUG_WRITE_ENTIRE_FILE(name) void name(game::thread_context* thread, char* file_name, memory_index memory_size, void* memory)
typedef DEBUG_WRITE_ENTIRE_FILE(debug_write_entire_file_def);

#define DEBUG_FREE_FILE_MEMORY(name) void name(game::thread_context* thread, void* memory)
typedef DEBUG_FREE_FILE_MEMORY(debug_free_file_memory_def);

#define DEBUG_GOTO_PLAYBACK(name) void name()
typedef DEBUG_GOTO_PLAYBACK(debug_goto_playback_def);
#endif

}//namespace game

global_variable game::debug_goto_playback_def* _goto_playback_func = nullptr;

#if GAME_SLOWMODE
#define assert(expression) {if(!(expression)){game::debug_break(); if(_goto_playback_func){_goto_playback_func();}}}
#else
#define assert(expression) {};
#endif

namespace game
{

struct memory_space
{
	u8* base;
	memory_index max_memory;
	memory_index used_memory;
};

internal
void _initialize_memory_space(memory_space* space, u8* base, memory_index max_memory)
{
	space->base = base;
	space->max_memory = max_memory;
	space->used_memory = 0;
}

#define _push_struct(space, struct) (struct*)_push_size(space, sizeof(struct));
#define _push_array(space, type, num_elements) (type*)_push_size(space, sizeof(type)*num_elements);

internal
void* _push_size(memory_space* space, memory_index size)
{
	assert((space->used_memory + size) <= space->max_memory);
	void* result = (void*)(space->base + space->used_memory);
	space->used_memory += size;
	return result;
}

}//namespace game

#include "math.h"
#include "game_input.h"
#include "game_tilemap.h"
#include "game_random.h"

namespace game
{

struct game_memory //NOTE(staffan): memory REQUIRES to be initialized to zero
{
	memory_index permanent_storage_size;
	void* permanent_storage;

	memory_index transient_storage_size;
	void* transient_storage;

#if GAME_INTERNAL
	debug_read_entire_file_def* debug_read_entire_file;
	debug_write_entire_file_def* debug_write_entire_file;
	debug_free_file_memory_def* debug_free_file_memory;
	debug_goto_playback_def* debug_goto_playback;
#endif

	bool is_initialized;
};

struct render_output
{
	void* memory; //depth buffer starts at (u8*)memory + pitch*height
	i32 height;
	i32 width;
	i32 pitch;
	i32 depth_buffer_pitch;
};

struct audio_output
{
	u16* memory;
	i32 sample_rate;
	i32 num_samples_to_fill;
};

struct game_time
{
	f32 t;
	f32 dt;
};

#define GAME_UPDATE_AND_RENDER(name) void name(thread_context* thread, game_memory* memory, game_time time, input_state input, render_output& render_output)
typedef GAME_UPDATE_AND_RENDER(update_and_render_def);
GAME_UPDATE_AND_RENDER(update_and_render_stub)
{
}

#define GAME_FILL_AUDIO_OUTPUT(name) void name(thread_context* thread, game_memory* memory, audio_output& audio_output)
typedef GAME_FILL_AUDIO_OUTPUT(fill_audio_output_def);
GAME_FILL_AUDIO_OUTPUT(fill_audio_output_stub)
{
}

///
struct world
{
	tile_map* tile_map;
};

struct loaded_bitmap
{
	u32* pixels;

	i32 width;
	i32 height;

	i32 offset_x;
	i32 offset_y;
};

struct triangle
{
	i32 indicies[3];
};

struct loaded_mesh
{
	v3* vertices;
	triangle* faces;

	i32 num_vertices;
	i32 num_faces;
};

struct game_state
{
	memory_space mesh_space;
	memory_space world_space;
	world* world;

	math::v2 player_velocity;
	tile_map_position player_position;
	loaded_bitmap player_bitmap;

	loaded_bitmap test_bitmap;

	bool should_rotate;
	f32 rotation;

	v3 camera_position;
	v3 camera_direction;

	loaded_mesh test_mesh;
};

} //namespace game