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
	uint8 half_transitions;
};

struct input_state
{
	button_state buttons[LAST];

	short mouse_x;
	short mouse_y;
	short mouse_delta_x;
	short mouse_delta_y;

	short window_width;
	short window_height;
};

} //namespace game