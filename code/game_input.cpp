namespace input
{
global_variable game::input_state* this_frame = nullptr;

internal void new_frame(game::input_state* input)
{
	this_frame = input;
}

internal inline bool button_pressed(game::buttons button_code)
{
	game::button_state* button = &this_frame->buttons[button_code];
	return (button->half_transitions > (uint8_t)button->was_down) ? true : false;
}

internal inline bool button_released(game::buttons button_code)
{
	game::button_state* button = &this_frame->buttons[button_code];
	return (button->half_transitions > (uint8_t)!button->was_down) ? true : false;
}

internal inline bool button_held(game::buttons button_code)
{
	game::button_state* button = &this_frame->buttons[button_code];
	return (button->half_transitions%2 == !button->was_down) ? true : false;
}

internal inline short mouse_delta_x()
{
	return this_frame->mouse_delta_x;
}

internal inline short mouse_delta_y()
{
	return this_frame->mouse_delta_y;
}
} //namespace input