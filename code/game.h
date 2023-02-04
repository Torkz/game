#include <stdint.h> //todo(staffan): remove this dependency?

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
#endif
//
//	/Compilers
//


#define global_variable static
#define internal static
#define local_persist static

#define PI 3.14159265359f
#define TAU PI*2.0f

#define memory_index size_t

#if GAME_SLOWMODE
#define assert(expression) {if(!(expression)){*(uint8_t*)0 = 0;}}
#else
#define assert(expression) {};
#endif

#define kilobytes(value) ((value)*1024)
#define megabytes(value) (kilobytes(value)*1024)
#define gigabytes(value) (megabytes(value)*1024)
#define terabytes(value) (gigabytes(value)*1024)


#include "game_input.h"

namespace game
{
struct memory_space
{
	uint8_t* base;
	memory_index max_memory;
	memory_index used_memory;
};

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
#include "game_tilemap.h"
#include "game_random.h"

namespace game
{
struct thread_context
{
	int placeholder;
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
#endif

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
#endif

	bool is_initialized;
};

struct render_output
{
	void* memory;
	int height;
	int width;
	int pitch;
};

struct audio_output
{
	uint16_t* memory;
	int sample_rate;
	int num_samples_to_fill;
};

struct game_time
{
	float t;
	float dt;
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
	uint32_t* pixels;

	int32_t width;
	int32_t height;
};

struct game_state
{
	memory_space world_space;

	world* world;
	tile_map_position player_position;

	uint32_t* pixels;
	loaded_bitmap test_bitmap;
};

//forward declared "private" functions
}