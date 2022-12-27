#include <stdint.h> //todo(staffan): remove this dependency?
#include "game_input.h"
#include "game_tilemap.h"

#define global_variable static
#define internal static
#define local_persist static

#define PI 3.14159265359f
#define TAU PI*2.0f

#if GAME_SLOWMODE
#define assert(expression) {if(!(expression)){*(uint8_t*)0 = 0;}}
#else
#define assert(expression) {};
#endif

#define kilobytes(value) ((value)*1024)
#define megabytes(value) (kilobytes(value)*1024)
#define gigabytes(value) (megabytes(value)*1024)
#define terabytes(value) (gigabytes(value)*1024)

namespace game
{
struct game_memory //NOTE(staffan): memory REQUIRES to be initialized to zero
{
	uint64_t permanent_storage_size;
	void* permanent_storage;

	uint64_t transient_storage_size;
	void* transient_storage;

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

#define GAME_UPDATE_AND_RENDER(name) void name(game_memory* memory, game_time time, input_state input, render_output& render_output)
typedef GAME_UPDATE_AND_RENDER(update_and_render_def);
GAME_UPDATE_AND_RENDER(update_and_render_stub)
{
}

#define GAME_FILL_AUDIO_OUTPUT(name) void name(game_memory* memory, audio_output& audio_output)
typedef GAME_FILL_AUDIO_OUTPUT(fill_audio_output_def);
GAME_FILL_AUDIO_OUTPUT(fill_audio_output_stub)
{
}

///
struct world
{
	tile_map* tile_map;
};

struct game_state
{
	world* world;
	tile_map_position player_position;
};

//forward declared "private" functions
}