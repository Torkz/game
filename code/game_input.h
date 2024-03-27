#pragma once

namespace game
{
enum buttons
{
	MOUSE_LEFT = 0, MOUSE_RIGHT,
	A, S, D, W, SPACEBAR,
	LAST
};

struct button_state
{
	bool was_down;
	u8 half_transitions;
};

struct input_state
{
	button_state buttons[LAST];

	i16 mouse_x;
	i16 mouse_y;
	i16 mouse_delta_x;
	i16 mouse_delta_y;

	i16 window_width;
	i16 window_height;
};

} //namespace game