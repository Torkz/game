#include <math.h> //todo(staffan): get rid of this.

#include "game.h"
#include "input.cpp"

namespace game
{
extern "C" GAME_UPDATE_AND_RENDER(update_and_render)
{
	assert(memory->permanent_storage_size >= sizeof(game::game_state));
	game_state* game_state = (game::game_state*)memory->permanent_storage;
	if(!memory->is_initialized)
	{
		game_state->tone_herz = 261.63f;
		game_state->volume = 2000.0f;

		memory->is_initialized = true;
	}

	input::new_frame(&input);

	_handle_input(game_state, time);
	_update_sine_wave(game_state, time);
	_render_gradient(game_state, render_output);
}

extern "C" GAME_FILL_AUDIO_OUTPUT(fill_audio_output)
{
	game_state* game_state = (game::game_state*)memory->permanent_storage;
	_fill_sine_wave_buffer(game_state, audio_output);
}

internal void _handle_input(game_state* game_state, game_time time)
{
	if (input::button_held(MOUSE_LEFT))
	{
		game_state->x_offset -= input::this_frame->mouse_delta_x;
		game_state->y_offset -= input::this_frame->mouse_delta_y;
	}
	else
	{
		if (input::button_held(W))
		{
			game_state->y_offset += 2;
		}
		if (input::button_held(S))
		{
			game_state->y_offset -= 2;
		}
		if (input::button_held(A))
		{
			game_state->x_offset += 2;
		}
		if (input::button_held(D))
		{
			game_state->x_offset -= 2;
		}
	}
}

internal void _update_sine_wave(game_state* game_state, game_time time)
{
	bool holding_w = input::button_held(W);
	bool holding_s = input::button_held(S);
	bool holding_a = input::button_held(A);
	bool holding_d = input::button_held(D);

	bool moving = (holding_w && !holding_s)
				|| (holding_s && !holding_w)
				|| (holding_a && !holding_d)
				|| (holding_d && !holding_a);

	float low_tone = 261.63f;
	float high_tone = 523.25f;
	float tone_increment = (high_tone-low_tone)/(1.0f/10.0f);
	if(moving)
	{
		game_state->tone_herz += tone_increment*time.dt;
		game_state->tone_herz = game_state->tone_herz > high_tone ? high_tone : game_state->tone_herz;
	}
	else
	{
		game_state->tone_herz -= tone_increment*time.dt;
		game_state->tone_herz = game_state->tone_herz < low_tone ? low_tone : game_state->tone_herz;
	}

	float volume_width_scalar = 0.5f * (float)input::this_frame->mouse_x/(float)input::this_frame->window_width;
	float volume_height_scalar = 0.5f * ((float)input::this_frame->mouse_y)/(float)input::this_frame->window_height;
	float volume_scalar = volume_width_scalar + volume_height_scalar;
	float volume_low = 1000.0f;
	float volume_high = 6000.0f;
	game_state->volume = (((1.0f-volume_scalar)*volume_low) + (volume_scalar*(volume_high-volume_low)));
}

internal void _render_gradient(game_state* game_state, render_output& render_output)
{
	// uint8_t blue_x = (uint8_t)(128.0f * ((float)input::this_frame->mouse_x/(float)input::this_frame->window_width));
	// uint8_t blue_y = (uint8_t)(127.0f * (((float)input::this_frame->mouse_y/(float)input::this_frame->window_height)));
	// uint8_t blue = blue_x+blue_y;

	uint8_t* row = (uint8_t*)render_output.memory;
	for (int y = 0; y < render_output.height; ++y)
	{
		uint8_t* pixel = (uint8_t*)row;
		for (int x = 0; x < render_output.width; ++x)
		{
			*pixel = 0;//blue; //blue
			++pixel;

			*pixel = (uint8_t)(y+game_state->y_offset); //green
			++pixel;

			*pixel = (uint8_t)(x+game_state->x_offset); //red
			++pixel;

			*pixel = 0;
			++pixel;
		}

		row += render_output.pitch;
	}
}

internal void _fill_sine_wave_buffer(game_state* game_state, audio_output& audio_output)
{	
	float wave_period = (float)audio_output.sample_rate/game_state->tone_herz;
	float t_sine_increment = TAU*1.0f/wave_period;

	uint16_t* current_sample = audio_output.memory;
	for(
		int i=0;
		i < audio_output.num_samples_to_fill;
		++i
	)
	{
		float sine_value = sinf(game_state->t_sine);
		uint16_t sample_value = (uint16_t)(game_state->volume*sine_value);

		*current_sample++ = sample_value; //left
		*current_sample++ = sample_value; //right

		game_state->t_sine += t_sine_increment;
		if(game_state->t_sine > TAU)
		{
			game_state->t_sine -= TAU;
		}
	}
}
}//namespace game