#pragma once
#include "win_input_codes.h"

#include <windows.h>
#include <strsafe.h>
#include <xaudio2.h>
#include <stdio.h>

struct win_bitmap_buffer
{
	BITMAPINFO info;
	void* memory;
	int width;
	int height;
	int bytes_per_pixel;
	int pitch;
};

struct game_code
{
	FILETIME file_last_write_time;
	HMODULE dll_handle;

	game::update_and_render_def* update_and_render;
	game::fill_audio_output_def* fill_audio_output;

	bool is_valid;
};

#if GAME_INTERNAL
#include <string.h>
#include <setjmp.h>
struct frame_recording
{
	game::input_state input;
	game::game_time time;
};

struct win_playback_state
{
	uint16 num_recording_frames = 60;
	uint16 num_recorded_frames = 0;
	frame_recording* frame_recording_start = nullptr;
	uint8* game_state_recording_start = nullptr;
	uint16 frame_recording_index = 0;

	bool playback = false;
	bool stopping_playback = false;
	uint16 playback_started_frame_recording_index = 0;
	uint16 playback_current_frame_recording_index = 0;
};
#endif

struct win_state
{
#if GAME_INTERNAL
	win_playback_state* playback_state;
#endif
};

//forward declared functions
LRESULT CALLBACK main_window_callback(HWND hwnd, UINT message, WPARAM param1, LPARAM param2);
inline internal LARGE_INTEGER _windows_time();
inline internal float32 _time_elapsed(LARGE_INTEGER begin, LARGE_INTEGER end);
inline internal game::button_state* _button_state_from_key_code(game::input_state* game_input, uint32 key_code);
inline internal int _string_length(char* string);
inline internal void _combine_two_strings(char* destination, char* string_a, int string_a_length, char* string_b, int string_b_length);
internal void _load_game_code(game_code* game_code, char* dll_file_path, char* temp_dll_file_path);
internal void _unload_game_code(game_code* game_code);
internal void _process_window_messages(HWND window, game::input_state* current_input, game::input_state* previous_input, game::game_memory* game_memory, win_state* win_state);
internal void _resize_dib_section(win_bitmap_buffer* bitmap_buffer, int width, int height);
internal void _update_window(HDC device_context, RECT* window_rect, win_bitmap_buffer* buffer);
internal void _initialize_xaudio2(WORD num_channels, WORD bits_per_sample, DWORD sample_rate, float32 buffer_duration);
internal void _post_error(const char* function);

#if GAME_INTERNAL
internal void _start_playback(win_playback_state* playback_state, game::game_memory* memory);
internal void _record_frame(
	win_playback_state* playback_state,
	game::input_state input,
	game::game_time time,
	game::game_memory* memory
);
internal void _playback_frame(
	win_playback_state* playback_state,
	game::input_state* input,
	game::game_time* time,
	game::game_memory* memory
);

DEBUG_FREE_FILE_MEMORY(debug_free_file_memory);
DEBUG_READ_ENTIRE_FILE(debug_read_entire_file);
DEBUG_WRITE_ENTIRE_FILE(debug_write_entire_file);
DEBUG_GOTO_PLAYBACK(debug_goto_playback);
#endif