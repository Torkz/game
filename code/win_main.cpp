#include "game.h"
#include "win_main.h"

global_variable bool _running = true;
global_variable win_bitmap_buffer _bitmap_buffer;

#define XAUDIO2_NUM_BUFFERS 3
global_variable IXAudio2* _xaudio2_engine = nullptr;
global_variable IXAudio2MasteringVoice* _xaudio2_mastering_voice = nullptr;
global_variable IXAudio2SourceVoice* _xaudio2_source_voice = nullptr;
global_variable UINT32 _xaudio2_audio_data_buffer_size;
global_variable BYTE* _xaudio2_audio_data;

global_variable i64 _performance_counter_frequency;

#if GAME_INTERNAL
global_variable jmp_buf _forced_playback_jmp_buf;
#endif

int WinMain(
	HINSTANCE instance,
	HINSTANCE previous_instance,
	char*     command_line,
	int       show_code
)
{
	char exe_file_path[MAX_PATH];
	GetModuleFileNameA(0, exe_file_path, MAX_PATH);

	char* exe_directory_ptr = exe_file_path;
	int exe_directory_path_size = 0;
	int step_counter = 0;
	for(char* current = exe_file_path; *current; ++current)
	{
		if(*current == '\\')
		{
			exe_directory_ptr = current+1;
			exe_directory_path_size += step_counter+1;
			step_counter = 0;
		}
		else
		{
			++step_counter;
		}
	}

	char* dll_file_name = "game.dll";
	char dll_file_path[MAX_PATH];
	_combine_two_strings(dll_file_path,
		exe_file_path, exe_directory_path_size,
		dll_file_name, _string_length(dll_file_name)
	);

	char* temp_dll_file_name = "game_temp.dll";
	char temp_dll_file_path[MAX_PATH];
	_combine_two_strings(temp_dll_file_path,
		exe_file_path, exe_directory_path_size,
		temp_dll_file_name, _string_length(temp_dll_file_name)
	);

	game_code game_code = {};
	_load_game_code(&game_code, dll_file_path, temp_dll_file_path);
	if(!game_code.is_valid)
	{
		//todo(staffan): tell user that game.dll loading failed.
		return 0;
	}

	//todo(staffan): get this from windows.
	int monitor_update_frequency = 60;
	int game_update_frequency = monitor_update_frequency/2;
	f32 target_dt = 1.0f/(f32)game_update_frequency;

	LARGE_INTEGER performance_frequency_result;
	QueryPerformanceFrequency(&performance_frequency_result);
	_performance_counter_frequency = performance_frequency_result.QuadPart;

	WNDCLASSEXA window_class = {};
	window_class.cbSize = sizeof(WNDCLASSEX);
	window_class.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	window_class.lpfnWndProc = main_window_callback;
	window_class.lpszClassName = "GameWindowClass";
	window_class.hInstance = instance;

	if (!RegisterClassExA(&window_class))
	{
		_post_error("RegisterClassEx");
		return 0;
	}

	int wanted_width = 960; //1920/2
	int wanted_height = 540; //1080/2
	DWORD window_style = WS_OVERLAPPEDWINDOW|WS_VISIBLE;
	RECT wanted_window_rect = {
		0, 0, wanted_width, wanted_height
	};

	if (!AdjustWindowRect(&wanted_window_rect, window_style, false))
	{
		_post_error("AdjustWindowRect");
		return 0;
	}

	HWND window = CreateWindowA(
		window_class.lpszClassName,
		"Game",
		window_style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		wanted_window_rect.right-wanted_window_rect.left,
		wanted_window_rect.bottom-wanted_window_rect.top,
		0,
		0,
		instance,
		0
	);

	if (window == NULL)
	{
		_post_error("CreateWindow");
		return 0;
	}

	win_state win_state;

	bool stereo = true;
	WORD num_channels = stereo ? 2 : 1;
	WORD bits_per_sample = 16;
	DWORD sample_rate = 48000;
	//todo(staffan): figure out audio timings with framerate.
	f32 buffer_duration = 1.0f/60.0f; //seconds
	// float32 buffer_duration = target_dt; //seconds
	_initialize_xaudio2(num_channels, bits_per_sample, sample_rate, buffer_duration);

	u32 num_samples_per_buffer = (_xaudio2_audio_data_buffer_size*8)/(bits_per_sample*2);

	int audio_buffer_index = 0;
	BYTE audio_buffer[((2*16)/8)*48000*2]; //todo(staffan): revisit size.

#if GAME_INTERNAL
	LPVOID address_location = (LPVOID)terabytes((u64)2);
#else
	LPVOID address_location = 0;
#endif

	game::game_memory memory;
	memory.permanent_storage_size = megabytes(32);
	memory.transient_storage_size = gigabytes(1);

	u64 audio_memory_size = _xaudio2_audio_data_buffer_size*XAUDIO2_NUM_BUFFERS;
	u64 all_memory_size = audio_memory_size+memory.permanent_storage_size+memory.transient_storage_size;

#if GAME_INTERNAL
	win_playback_state playback_state;
	playback_state.num_recording_frames = 60;
	playback_state.num_recorded_frames = 0;
	playback_state.frame_recording_index = 0;
	playback_state.playback = false;
	playback_state.stopping_playback = false;
	playback_state.playback_started_frame_recording_index = 0;
	playback_state.playback_current_frame_recording_index = 0;
	win_state.playback_state = &playback_state;

	u64 frame_recording_size = sizeof(frame_recording)*playback_state.num_recording_frames;
	u64 game_state_recording_size = memory.permanent_storage_size*playback_state.num_recording_frames;
	u64 recording_memory_size = frame_recording_size + game_state_recording_size;
	all_memory_size += recording_memory_size;
#endif

	void* allocated_memory = VirtualAlloc(address_location, all_memory_size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

	if(allocated_memory == 0)
	{
		//todo(staffan): tell user memory alloc failed + how much memory is needed.
		_post_error("VirtualAlloc");
		return 0;
	}

	_xaudio2_audio_data = (BYTE*)allocated_memory;
	memory.permanent_storage = (u8*)allocated_memory + audio_memory_size;
	memory.transient_storage = (u8*)allocated_memory + audio_memory_size + memory.permanent_storage_size;

#if GAME_INTERNAL
	playback_state.frame_recording_start = (frame_recording*)((u8*)memory.transient_storage + memory.transient_storage_size);
	playback_state.game_state_recording_start = (u8*)playback_state.frame_recording_start + frame_recording_size;
#endif

#if GAME_INTERNAL
	memory.debug_read_entire_file = &debug_read_entire_file;
	memory.debug_write_entire_file = &debug_write_entire_file;
	memory.debug_free_file_memory = &debug_free_file_memory;
	memory.debug_goto_playback = &debug_goto_playback;
#endif

	_resize_dib_section(&_bitmap_buffer, wanted_width, wanted_height);
	//xaudio2 test
	HRESULT result;
	if( FAILED(result = _xaudio2_source_voice->Start(0, XAUDIO2_COMMIT_NOW)))
	{
		//todo(staffan): add logging
		return 0;
	}
	
	game::thread_context thread = {};

	game::game_time game_time = {0};
	game::input_state last_frame_input = {0};
#if 0
	float32 time_since_last_print = 0.0f;
#endif

	LARGE_INTEGER last_time = _windows_time();
#if 1
	i64 last_cycle_count = __rdtsc();
#endif
	while (_running)
	{
		_load_game_code(&game_code, dll_file_path, temp_dll_file_path);

		game::input_state game_input;
		_process_window_messages(window, &game_input, &last_frame_input, &memory, &win_state);

#if GAME_INTERNAL
		if(playback_state.playback)
		{
			_playback_frame(&playback_state, &game_input, &game_time, &memory);
		}
#endif

		game::render_output game_render_output = {};
		game_render_output.memory = _bitmap_buffer.memory;
		game_render_output.height = _bitmap_buffer.height;
		game_render_output.width = _bitmap_buffer.width;
		game_render_output.pitch = _bitmap_buffer.pitch;
		game_render_output.depth_buffer_pitch = _bitmap_buffer.depth_buffer_pitch;

#if GAME_INTERNAL
		if(setjmp(_forced_playback_jmp_buf)==0)
		{
			game_code.update_and_render(&thread, &memory, game_time, game_input, game_render_output);
		}
		else //forced playback
		{
			_record_frame(&playback_state, game_input, game_time, &memory);
			_start_playback(&playback_state, &memory);
		}
#else
		game_code.update_and_render(&thread, &memory, game_time, game_input, game_render_output);
#endif

#if GAME_INTERNAL
		if(!playback_state.playback)//frame_recording
		{
			_record_frame(&playback_state, game_input, game_time, &memory);
		}
#endif

		{ //render
			HDC device_context = GetDC(window);
			RECT client_rect;
			GetClientRect(window, &client_rect);
			_update_window(device_context, &client_rect, &_bitmap_buffer);
			ReleaseDC(window, device_context);
		}
		{ //xaudio2
#if 0
			bool printed = false;
#endif
			XAUDIO2_VOICE_STATE voice_state = {};
			_xaudio2_source_voice->GetState(&voice_state, XAUDIO2_VOICE_NOSAMPLESPLAYED);
			int num_audio_buffers_to_fill = XAUDIO2_NUM_BUFFERS-voice_state.BuffersQueued;

			game::audio_output game_audio_output = {};
			game_audio_output.memory = (u16*)audio_buffer;
			game_audio_output.sample_rate = sample_rate;
			game_audio_output.num_samples_to_fill = num_samples_per_buffer*num_audio_buffers_to_fill;

			game_code.fill_audio_output(&thread, &memory, game_audio_output);

			u16* input_sample = (u16*)audio_buffer;
			for(
				int i = 0;
				i < num_audio_buffers_to_fill;
				++i
			)
			{
#if 0
				if (!printed)
				{
					printed = true;

					float32 time_since_last_fill = game_time.t - time_since_last_print;
					time_since_last_print = game_time.t;

					char buffer[256];
					snprintf(buffer, 256, "time_since_last_fill:%.1fms num_buffers_filled:%i\n", 1000.0f*time_since_last_fill, num_audio_buffers_to_fill);
					OutputDebugStringA(buffer);
				}
#endif

				BYTE* audio_data_pointer = _xaudio2_audio_data + (_xaudio2_audio_data_buffer_size*audio_buffer_index);
				//todo(staffan): make this faster.
				u16* output_sample = (u16*)audio_data_pointer;
				for(
					u32 sample_index = 0;
					sample_index < num_samples_per_buffer;
					++sample_index
				)
				{
					*output_sample++ = *input_sample++; //left
					*output_sample++ = *input_sample++; //right
				}
				///todo

				XAUDIO2_BUFFER buffer = {};
				buffer.AudioBytes = _xaudio2_audio_data_buffer_size;
				buffer.pAudioData = audio_data_pointer;
				_xaudio2_source_voice->SubmitSourceBuffer(&buffer);

				++audio_buffer_index = audio_buffer_index%XAUDIO2_NUM_BUFFERS;
			}
		}

		LARGE_INTEGER work_time = _windows_time();
		f32 work_time_elapsed = _time_elapsed(last_time, work_time);
		if (work_time_elapsed < target_dt)
		{
			// Sleep((int)((target_dt-work_time_elapsed)*1000.0f));
			do
			{
				work_time = _windows_time();
			} while (_time_elapsed(last_time, work_time) < target_dt);
		}
		else
		{
			OutputDebugStringA("Missed target_dt\n");
		}

		LARGE_INTEGER end_time = _windows_time();
#if 0
		int64 end_cycle_count = __rdtsc();
		int64 cycles_elapsed = end_cycle_count - last_cycle_count;
		last_cycle_count = end_cycle_count;

		float32 ms_elapsed = 1000.0f*game_time.dt;
		float32 fps = (float32)((float32)_performance_counter_frequency / (float32)(end_time.QuadPart - last_time.QuadPart));
		float32 mcpf = (float32)((float32)cycles_elapsed / (1000.0f * 1000.0f));

		char buffer[256];
		snprintf(buffer, 256, "ms/frame: %fms fps:%f mc:%f dt:%.5f t:%.2f\n", ms_elapsed, fps, mcpf, game_time.dt, game_time.t);
		OutputDebugStringA(buffer);
#endif
#if GAME_INTERNAL
		game_time.dt = _time_elapsed(last_time, end_time);
		if(game_time.dt > target_dt)
		{
			game_time.dt = target_dt;
		}
		game_time.t += game_time.dt;
#else
		game_time.dt = _time_elapsed(last_time, end_time);
		game_time.t += game_time.dt;
#endif
		last_time = end_time;
	}

	if( FAILED(result = _xaudio2_source_voice->Stop(0)))
	{
		return 0;
	}

	return 0;
}

inline internal int _string_length(char* string)
{
	int result = 0;
	for(char* current = string; *current; ++current)
	{
		++result;
	}
	return result;
}

inline internal void _combine_two_strings(char* destination, char* string_a, int string_a_length, char* string_b, int string_b_length)
{
	for(int i = 0; i<string_a_length; ++i)
	{
		destination[i] = string_a[i];
	}
	for(int i = 0; i<string_b_length; ++i)
	{
		destination[string_a_length+i] = string_b[i];
	}
}

LRESULT CALLBACK main_window_callback(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;

	switch (message)
	{
		case WM_CLOSE:
		{
			_running = false;
		} break;
		case WM_CREATE:
		{
		} break;
		case WM_DESTROY:
		{
		} break;
		case WM_MOVE:
		{
		} break;
		case WM_SIZE:
		{
		} break;
		case WM_PAINT:
		{
			PAINTSTRUCT paint;
			HDC device_context = BeginPaint(window, &paint);
			RECT client_rect;
			GetClientRect(window, &client_rect);
			_update_window(device_context, &client_rect, &_bitmap_buffer);
			EndPaint(window, &paint);
		} break;
		case WM_ACTIVATE:
		{
			OutputDebugStringA("WM_ACTIVATE callback\n");
		} break;
		// keys
		case WM_KEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYDOWN:
		case WM_SYSKEYUP:
		{
			assert(false);
		} break;
		// /keys
		// mouse
		case WM_MOUSEMOVE:
		{
			assert(false);
		} break;
		default:
		{
			result = DefWindowProc(window, message, wparam, lparam);
		} break;
	}

	return result;
}

inline internal LARGE_INTEGER _windows_time()
{
	LARGE_INTEGER counter;
	QueryPerformanceCounter(&counter);
	return counter;
}

inline internal f32 _time_elapsed(LARGE_INTEGER begin, LARGE_INTEGER end)
{
	return (f32)(end.QuadPart - begin.QuadPart) / (f32)_performance_counter_frequency;
}

inline internal game::button_state* _button_state_from_key_code(game::input_state* game_input, u32 key_code)
{
	if (key_code == BUTTON_A)
	{
		return &game_input->buttons[game::buttons::A];
	}
	else if (key_code == BUTTON_S)
	{
		return &game_input->buttons[game::buttons::S];
	}
	else if (key_code == BUTTON_D)
	{
		return &game_input->buttons[game::buttons::D];
	}
	else if (key_code == BUTTON_W)
	{
		return &game_input->buttons[game::buttons::W];
	}
	else if (key_code == BUTTON_Q)
	{
		return &game_input->buttons[game::buttons::Q];
	}
	else if (key_code == BUTTON_E)
	{
		return &game_input->buttons[game::buttons::E];
	}
	else if(key_code == BUTTON_SPACEBAR)
	{
		return &game_input->buttons[game::buttons::SPACEBAR];
	}

	return nullptr;
}

inline internal bool _should_load_game_code(game_code* game_code, FILETIME file_last_write_time)
{
	return !game_code->is_valid || (CompareFileTime(&game_code->file_last_write_time, &file_last_write_time)!=0);
}

internal void _load_game_code(game_code* game_code, char* dll_file_path, char* temp_dll_file_path)
{
	WIN32_FILE_ATTRIBUTE_DATA attribute_data;
	GetFileAttributesExA(dll_file_path, GetFileExInfoStandard, (LPVOID)&attribute_data);

	if(_should_load_game_code(game_code, attribute_data.ftLastWriteTime))
	{
		if(game_code->dll_handle)
		{
			FreeLibrary(game_code->dll_handle);
		}

		CopyFileA(dll_file_path, temp_dll_file_path, false);

		game_code->dll_handle = LoadLibraryA(temp_dll_file_path);
		if(game_code->dll_handle)
		{
			game_code->file_last_write_time = attribute_data.ftLastWriteTime;
			game_code->update_and_render = (game::update_and_render_def*)GetProcAddress(game_code->dll_handle, "update_and_render");
			game_code->fill_audio_output = (game::fill_audio_output_def*)GetProcAddress(game_code->dll_handle, "fill_audio_output");

			game_code->is_valid = (game_code->update_and_render && game_code->fill_audio_output);
		}

		if(!game_code->is_valid)
		{
			game_code->update_and_render = game::update_and_render_stub;
			game_code->fill_audio_output = game::fill_audio_output_stub;
		}
	}
}

internal void _unload_game_code(game_code* game_code)
{
	if(game_code->dll_handle)
	{
		FreeLibrary(game_code->dll_handle);
		game_code->dll_handle = 0;
	}

	game_code->update_and_render = game::update_and_render_stub;
	game_code->fill_audio_output = game::fill_audio_output_stub;
	game_code->is_valid = false;
}

internal void _process_window_messages(HWND window, game::input_state* current_input, game::input_state* previous_input, game::game_memory* game_memory, win_state* win_state)
{
		HDC device_context = GetDC(window);
		RECT client_rect;
		GetClientRect(window, &client_rect);
		ReleaseDC(window, device_context);
		int window_width = client_rect.right - client_rect.left;
		int window_height = client_rect.bottom - client_rect.top;

		current_input->window_width = (i16)window_width;
		current_input->window_height = (i16)window_height;
		current_input->mouse_x = previous_input->mouse_x;
		current_input->mouse_y = previous_input->mouse_y;
		for (int i=0; i<game::buttons::LAST; ++i)
		{
			game::button_state* this_frame = &current_input->buttons[i];
			game::button_state* last_frame = &previous_input->buttons[i];

			this_frame->was_down = (last_frame->half_transitions%2 == (u8)!last_frame->was_down) ? true : false;
			this_frame->half_transitions = 0;
		}

		MSG message;
		while(PeekMessage(&message, window, 0, 0, PM_REMOVE))
		{
			switch(message.message)
			{
				case WM_QUIT:
				{
					_running = false;
				}break;

				case WM_KEYDOWN:
				case WM_KEYUP:
				case WM_SYSKEYDOWN:
				case WM_SYSKEYUP:
				{
					u32 key_code = (u32)message.wParam;

					bool was_down = (message.lParam & (1 << 30)) != 0;
					bool is_down = (message.lParam & (1 << 31)) == 0;

					game::button_state* button_state = _button_state_from_key_code(current_input, key_code);
					if (button_state)
					{
						if(was_down != is_down)
						{
							++button_state->half_transitions;
						}
					}

					u32 alt_key_was_down = (message.lParam & (1 << 29));
					if(key_code == VK_F4 && is_down && alt_key_was_down)
					{
						_running = false;
					}
#if GAME_INTERNAL
					if(key_code == BUTTON_L && is_down && alt_key_was_down)
					{
						if(win_state->playback_state->playback)
						{
							win_state->playback_state->stopping_playback = !win_state->playback_state->stopping_playback;
						}
						else
						{
							_start_playback(win_state->playback_state, game_memory);
						}
					}
#endif
				} break;
				case WM_LBUTTONDOWN:
				{
					++current_input->buttons[game::buttons::MOUSE_LEFT].half_transitions;
					SetCapture(window);
				} break;
				case WM_LBUTTONUP:
				{
					++current_input->buttons[game::buttons::MOUSE_LEFT].half_transitions;
					ReleaseCapture();
				} break;
				case WM_RBUTTONDOWN:
				{
					++current_input->buttons[game::buttons::MOUSE_RIGHT].half_transitions;
					SetCapture(window);
				} break;
				case WM_RBUTTONUP:
				{
					++current_input->buttons[game::buttons::MOUSE_RIGHT].half_transitions;
					ReleaseCapture();
				} break;
				case WM_MOUSEMOVE:
				{
					POINTS coordinates = MAKEPOINTS(message.lParam);
					current_input->mouse_x = coordinates.x;
					current_input->mouse_y = coordinates.y;
				} break;
				case WM_ACTIVATE:
				{
					assert(false);
				} break;

				default:
				{
					TranslateMessage(&message);
					DispatchMessage(&message);
				} break;
			}
		}
		current_input->mouse_delta_x = current_input->mouse_x-previous_input->mouse_x;
		current_input->mouse_delta_y = current_input->mouse_y-previous_input->mouse_y;
		*previous_input = *current_input;
#if 0
		char buffer[256];
		snprintf(buffer, 256, "mouse:%i %i delta:%i %i window:%i %i\n", game_input.mouse_x, game_input.mouse_y, game_input.mouse_delta_x, game_input.mouse_delta_y, window_width, window_height);
		OutputDebugStringA(buffer);
#endif
}

internal void _resize_dib_section(win_bitmap_buffer* bitmap_buffer, int width, int height)
{
	if (bitmap_buffer->memory)
	{
		VirtualFree(bitmap_buffer->memory, 0, MEM_RELEASE);
	}

	bitmap_buffer->width = width;
	bitmap_buffer->height = height;
	bitmap_buffer->bytes_per_pixel = 4;
	bitmap_buffer->pitch = bitmap_buffer->width * bitmap_buffer->bytes_per_pixel;
	bitmap_buffer->depth_buffer_pitch = bitmap_buffer->width * sizeof(f32);

	bitmap_buffer->info.bmiHeader.biSize = sizeof(bitmap_buffer->info.bmiHeader);
	bitmap_buffer->info.bmiHeader.biWidth = bitmap_buffer->width;
	bitmap_buffer->info.bmiHeader.biHeight = -bitmap_buffer->height;
	bitmap_buffer->info.bmiHeader.biPlanes = 1;
	bitmap_buffer->info.bmiHeader.biBitCount = 32;
	bitmap_buffer->info.bmiHeader.biCompression = BI_RGB;

	int bitmap_memory_size = bitmap_buffer->width*bitmap_buffer->height * bitmap_buffer->bytes_per_pixel;
	int back_buffer_size = bitmap_buffer->width*bitmap_buffer->height * sizeof(f32);
	bitmap_buffer->memory = VirtualAlloc(0, bitmap_memory_size+back_buffer_size, MEM_COMMIT, PAGE_READWRITE);
}

internal void _update_window(HDC device_context, RECT* window_rect, win_bitmap_buffer* buffer)
{
	int offset_x = 10;
	int offset_y = 10;
	int window_width = window_rect->right - window_rect->left;
	int window_height = window_rect->bottom - window_rect->top;

	PatBlt(device_context, 0, 0, window_width, offset_y, BLACKNESS);
	PatBlt(device_context, 0, 0, offset_x, window_height, BLACKNESS);
	PatBlt(device_context, offset_x, offset_y+buffer->height, window_width, window_height, BLACKNESS);
	PatBlt(device_context, offset_x+buffer->width, 0, window_width, window_height, BLACKNESS);

	StretchDIBits(
		device_context,
		offset_x, offset_y, buffer->width, buffer->height,
		0, 0, buffer->width, buffer->height,
		buffer->memory,
		&buffer->info,
		DIB_RGB_COLORS,
		SRCCOPY
	);
}

internal void _initialize_xaudio2(WORD num_channels, WORD bits_per_sample, DWORD sample_rate, f32 buffer_duration)
{
	HRESULT result;
	if ( FAILED(result = XAudio2Create( &_xaudio2_engine, 0, XAUDIO2_DEFAULT_PROCESSOR)))
	{
		//todo(staffan): failed creating xaudio engine
		return;
	}

	if ( FAILED(result = _xaudio2_engine->CreateMasteringVoice(
		&_xaudio2_mastering_voice,
		XAUDIO2_MAX_AUDIO_CHANNELS,
		sample_rate, //todo(staffan): figure out what samplerate to use
		0,
		NULL,
		NULL,
		AudioCategory_GameEffects
	)))
	{
		//todo(staffan): failed creating master voice
		return;
	}

	WAVEFORMAT wave_format = {0};
	wave_format.wFormatTag = WAVE_FORMAT_PCM;
  	wave_format.nChannels = num_channels;
  	wave_format.nSamplesPerSec = sample_rate;
  	wave_format.nBlockAlign = (num_channels*bits_per_sample) / 8;
  	wave_format.nAvgBytesPerSec = wave_format.nBlockAlign*wave_format.nSamplesPerSec;

	PCMWAVEFORMAT wave_format_ex = {0};
	wave_format_ex.wf = wave_format;
	wave_format_ex.wBitsPerSample = bits_per_sample;

	if( FAILED( result = _xaudio2_engine->CreateSourceVoice(
		&_xaudio2_source_voice,
		(WAVEFORMATEX*)&wave_format_ex,
		0, //flags
		1.0f, //frequency_ratio
		nullptr, //callback
		nullptr, //send_list
		nullptr // effect_chain
	)))
	{
		//todo(staffan): failed creating source voice
		return;
	}

	_xaudio2_audio_data_buffer_size = (u32)((f32)wave_format.nAvgBytesPerSec*buffer_duration);
}

internal void _post_error(const char* function)
{
	unsigned long error_code = GetLastError();

    void* message_buffer;
	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error_code,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&message_buffer,
		0,
		NULL
	);

    void* display_buffer;
	display_buffer = (void*)LocalAlloc(
		LMEM_ZEROINIT,
		(lstrlenA((const char*)message_buffer) + lstrlenA(function) + 40) * sizeof(TCHAR)
	);
	StringCchPrintf(
		(LPTSTR)display_buffer,
		LocalSize(display_buffer) / sizeof(TCHAR),
		TEXT("%s faild with error %d: %s"),
		function, error_code, message_buffer
	);
	MessageBox(NULL, (LPCTSTR)display_buffer, TEXT("Error"), MB_OK);

	LocalFree(message_buffer);
	LocalFree(display_buffer);
}

inline internal
u32 safe_truncate_uint64(u64 value)
{
	assert(value <= 0xFFFFFFFF);
	u32 result = (u32)value;
	return result;
}

#if GAME_INTERNAL
internal void _start_playback(win_playback_state* playback_state, game::game_memory* memory)
{
	u16 start_playback_index;
	if(playback_state->num_recorded_frames==playback_state->num_recording_frames)
	{
		start_playback_index = playback_state->frame_recording_index;
	}
	else
	{
		start_playback_index = 0;
	}

	playback_state->playback = true;
	playback_state->playback_started_frame_recording_index = start_playback_index;
	playback_state->playback_current_frame_recording_index = start_playback_index;
}

internal void _record_frame(
	win_playback_state* playback_state,
	game::input_state input,
	game::game_time time,
	game::game_memory* memory
)
{
	frame_recording* current_frame_recording = playback_state->frame_recording_start+playback_state->frame_recording_index;
	current_frame_recording->input = input;
	current_frame_recording->time = time;
	u8* game_state_recording = playback_state->game_state_recording_start + playback_state->frame_recording_index*memory->permanent_storage_size;
	memcpy(game_state_recording, memory->permanent_storage, memory->permanent_storage_size);

	if(playback_state->num_recorded_frames < playback_state->num_recording_frames)
	{
		++playback_state->num_recorded_frames;
	}

	++playback_state->frame_recording_index = playback_state->frame_recording_index%playback_state->num_recording_frames;
}

internal void _playback_frame(
	win_playback_state* playback_state,
	game::input_state* input,
	game::game_time* time,
	game::game_memory* memory
)
{
	bool at_start = playback_state->playback_current_frame_recording_index==playback_state->playback_started_frame_recording_index;
	if(playback_state->stopping_playback && at_start)
	{
		playback_state->stopping_playback = false;
		playback_state->playback = false;
		return;
	}


	if(at_start)
	{
		u16 game_state_index = (playback_state->playback_current_frame_recording_index+1)%playback_state->num_recording_frames;
		u8* game_state_start = playback_state->game_state_recording_start + game_state_index*memory->permanent_storage_size;
		memcpy(memory->permanent_storage, game_state_start, memory->permanent_storage_size);
	}

	frame_recording* recording = playback_state->frame_recording_start+playback_state->playback_current_frame_recording_index;
	*input = recording->input;
	*time = recording->time;

	++playback_state->playback_current_frame_recording_index = playback_state->playback_current_frame_recording_index%playback_state->num_recorded_frames;
}

DEBUG_FREE_FILE_MEMORY(debug_free_file_memory)
{
	VirtualFree(memory, 0, MEM_RELEASE);
}

DEBUG_READ_ENTIRE_FILE(debug_read_entire_file)
{
	game::debug_read_file_results result = {};

	HANDLE handle = CreateFileA(file_name, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(handle == INVALID_HANDLE_VALUE)
	{
		//todo(staffan): add logging
		return result;
	}

	LARGE_INTEGER file_size;
	if(!GetFileSizeEx(handle, &file_size))
	{
		//todo(staffan): add logging
		CloseHandle(handle);
		return result;
	}

	u32 file_size_32 = safe_truncate_uint64(file_size.QuadPart);
	void* content = VirtualAlloc(0, file_size_32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
	if(content == 0)
	{
		//todo(staffan): add logging
		CloseHandle(handle);
		return result;
	}

	DWORD bytes_read;
	if(!ReadFile(handle, content, file_size_32, &bytes_read, 0) || file_size_32 != bytes_read)
	{
		//todo(staffan): add logging
		debug_free_file_memory(thread, content);
		CloseHandle(handle);
		return result;
	}

	CloseHandle(handle);

	result.content = content;
	result.size = file_size_32;

	return result;
}

DEBUG_WRITE_ENTIRE_FILE(debug_write_entire_file)
{
	HANDLE handle = CreateFileA(file_name, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if(handle == INVALID_HANDLE_VALUE)
	{
		//todo(staffan): add logging
		return;
	}

	DWORD size_to_write = safe_truncate_uint64(memory_size);
	DWORD size_written;
	if(!WriteFile(handle, memory, size_to_write, &size_written, 0) || size_written != size_to_write)
	{
		//todo(staffan): add logging
		CloseHandle(handle);
		return;
	}

	CloseHandle(handle);
}

DEBUG_GOTO_PLAYBACK(debug_goto_playback)
{
	longjmp(_forced_playback_jmp_buf, 1);
}
#endif