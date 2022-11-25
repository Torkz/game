#pragma once

//key codes from: https://learn.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes

#define MOUSE_BUTTON_LEFT			0x01
#define MOUSE_BUTTON_RIGHT			0x02
#define BUTTON_CANCEL				0x03
#define MOUSE_BUTTON_MIDDLE			0x04
#define MOUSE_BUTTON_X1				0x05
#define MOUSE_BUTTON_X2				0x06

#define BUTTON_BACKSPACE			0x08
#define BUTTON_TAB					0x09

#define BUTTON_CLEAR				0x0C
#define BUTTON_RETURN				0x0D

#define BUTTON_SHIFT				0x10
#define BUTTON_CTRL					0x11
#define BUTTON_ALT					0x12
#define BUTTON_PAUSE				0x13
#define BUTTON_CAPS_LOCK			0x14

#define BUTTON_ESCAPE				0x1B
//VK_ACCEPT	0x1E	IME accept

#define BUTTON_SPACEBAR				0x20
#define BUTTON_PAGE_UP				0x21
#define BUTTON_PAGE_DOWN			0x22
#define BUTTON_END					0x23
#define BUTTON_HOME					0x24
#define BUTTON_LEFT					0x25
#define BUTTON_UP					0x26
#define BUTTON_RIGHT				0x27
#define BUTTON_DOWN					0x28
#define BUTTON_SELECT				0x29
#define BUTTON_PRINT				0x2A
#define BUTTON_EXECUTE				0x2B
#define BUTTON_PRINT_SCREEN			0x2C
#define BUTTON_INSERT				0x2D
#define BUTTON_DELETE				0x2E
#define BUTTON_HELP					0x2F
#define BUTTON_0					0x30
#define	BUTTON_1					0x31
#define	BUTTON_2					0x32
#define	BUTTON_3					0x33
#define	BUTTON_4					0x34
#define	BUTTON_5					0x35
#define	BUTTON_6					0x36
#define	BUTTON_7					0x37
#define	BUTTON_8					0x38
#define	BUTTON_9					0x39

#define	BUTTON_A					0x41
#define	BUTTON_B					0x42
#define	BUTTON_C					0x43
#define	BUTTON_D					0x44
#define	BUTTON_E					0x45
#define	BUTTON_F					0x46
#define	BUTTON_G					0x47
#define	BUTTON_H					0x48
#define	BUTTON_I					0x49
#define	BUTTON_J					0x4A
#define	BUTTON_K					0x4B
#define	BUTTON_L					0x4C
#define	BUTTON_M					0x4D
#define	BUTTON_N					0x4E
#define	BUTTON_O					0x4F
#define	BUTTON_P					0x50
#define	BUTTON_Q					0x51
#define	BUTTON_R					0x52
#define	BUTTON_S					0x53
#define	BUTTON_T					0x54
#define	BUTTON_U					0x55
#define	BUTTON_V					0x56
#define	BUTTON_W					0x57
#define	BUTTON_X					0x58
#define	BUTTON_Y					0x59
#define	BUTTON_Z					0x5A
#define BUTTON_WIN_L				0x5B
#define BUTTON_WIN_R				0x5C
#define BUTTON_APP					0x5D

#define BUTTON_SLEEP				0x5F
#define	BUTTON_NUMPAD_0				0x60 
#define	BUTTON_NUMPAD_1				0x61
#define	BUTTON_NUMPAD_2				0x62
#define	BUTTON_NUMPAD_3				0x63
#define	BUTTON_NUMPAD_4				0x64
#define	BUTTON_NUMPAD_5				0x65
#define	BUTTON_NUMPAD_6				0x66
#define	BUTTON_NUMPAD_7				0x67
#define	BUTTON_NUMPAD_8				0x68
#define	BUTTON_NUMPAD_9				0x69
#define	BUTTON_MULTIPLY				0x6A
#define	BUTTON_ADD					0x6B
#define	BUTTON_SEPARATOR			0x6C
#define	BUTTON_SUBTRACT				0x6D
#define	BUTTON_DECIMAL				0x6E
#define	BUTTON_DIVIDE				0x6F

#define	BUTTON_F1					0x70
#define	BUTTON_F2					0x71
#define	BUTTON_F3					0x72
#define	BUTTON_F4					0x73
#define	BUTTON_F5					0x74
#define	BUTTON_F6					0x75
#define	BUTTON_F7					0x76
#define	BUTTON_F8					0x77
#define	BUTTON_F9					0x78
#define	BUTTON_F10					0x79
#define	BUTTON_F11					0x7A
#define	BUTTON_F12					0x7B
#define	BUTTON_F13					0x7C
#define	BUTTON_F14					0x7D
#define	BUTTON_F15					0x7E
#define	BUTTON_F16					0x7F
#define	BUTTON_F17					0x80
#define	BUTTON_F18					0x81
#define	BUTTON_F19					0x82
#define	BUTTON_F20					0x83
#define	BUTTON_F21					0x84
#define	BUTTON_F22					0x85
#define	BUTTON_F23					0x86
#define	BUTTON_F24					0x87

#define BUTTON_NUM_LOCK				0x90
#define BUTTON_SCROLL_LOCK			0x91
//0x92-96	OEM specific

#define BUTTON_SHIFT_L				0xA0
#define BUTTON_SHIFT_R				0xA1
#define BUTTON_CTRL_L				0xA2
#define BUTTON_CTRL_R				0xA3
#define BUTTON_ALT_L				0xA4
#define BUTTON_ALT_R				0xA5

#define BUTTON_BROWSER_BACK			0xA6
#define BUTTON_BROWSER_FORWARD		0xA7
#define BUTTON_BROWSER_REFRESH		0xA8
#define BUTTON_BROWSER_SEARCH		0xA9
#define BUTTON_BROWSER_FAVORITES	0xAB
#define BUTTON_BROWSER_HOME			0xAC
#define BUTTON_VOLUME_MUTE			0xAD
#define BUTTON_VOLUME_DOWN			0xAE
#define BUTTON_VOLUME_UP			0xAF
#define BUTTON_MEDIA_NEXT_TRACK		0xB0
#define BUTTON_MEDIA_PREV_TRACK		0xB1
#define BUTTON_MEDIA_STOP			0xB2
#define BUTTON_MEDIA_PLAY_PAUSE		0xB3
//BUTTON_LAUNCH: 0xB4-0xB7

#define BUTTON_OEM_1				0xBA
#define BUTTON_OEM_PLUS				0xBB
#define BUTTON_OEM_COMMA			0xBC
#define BUTTON_OEM_MINUS			0xBD
#define BUTTON_OEM_PERIOD			0xBE
#define BUTTON_OEM_2				0xBF
#define BUTTON_OEM_3				0xC0

#define BUTTON_OEM_4				0xDB
#define BUTTON_OEM_5				0xDC
#define BUTTON_OEM_6				0xDD
#define BUTTON_OEM_7				0xDE
#define BUTTON_OEM_8				0xDF

#define BUTTON_OEM_102				0xE2

//VK_PROCESSKEY	0xE5	IME PROCESS key

//VK_PACKET	0xE7	Used to pass Unicode characters as if they were keystrokes. The VK_PACKET key is the low word of a 32-bit Virtual Key value used for non-keyboard input methods. For more information, see Remark in KEYBDINPUT, SendInput, WM_KEYDOWN, and WM_KEYUP

//VK_ATTN	0xF6	Attn key
//VK_CRSEL	0xF7	CrSel key
//VK_EXSEL	0xF8	ExSel key
//VK_EREOF	0xF9	Erase EOF key
//VK_PLAY	0xFA	Play key
//VK_ZOOM	0xFB	Zoom key
//VK_NONAME	0xFC	Reserved
//VK_PA1	0xFD	PA1 key
#define BUTTON_OEM_CLEAR			0xFE

static const char* _button_id(int index)
{
	switch(index)
	{
		case MOUSE_BUTTON_LEFT: return("MOUSE_BUTTON_LEFT"); break;
		case MOUSE_BUTTON_RIGHT: return("MOUSE_BUTTON_RIGHT"); break;
		case BUTTON_CANCEL: return("BUTTON_CANCEL"); break;
		case MOUSE_BUTTON_MIDDLE: return("MOUSE_BUTTON_MIDDLE"); break;
		case MOUSE_BUTTON_X1: return("MOUSE_BUTTON_X1"); break;
		case MOUSE_BUTTON_X2: return("MOUSE_BUTTON_X2"); break;
		case BUTTON_BACKSPACE: return("BUTTON_BACKSPACE"); break;
		case BUTTON_TAB: return("BUTTON_TAB"); break;
		case BUTTON_CLEAR: return("BUTTON_CLEAR"); break;
		case BUTTON_RETURN: return("BUTTON_RETURN"); break;
		case BUTTON_SHIFT: return("BUTTON_SHIFT"); break;
		case BUTTON_CTRL: return("BUTTON_CTRL"); break;
		case BUTTON_ALT: return("BUTTON_ALT"); break;
		case BUTTON_PAUSE: return("BUTTON_PAUSE"); break;
		case BUTTON_CAPS_LOCK: return("BUTTON_CAPS_LOCK"); break;

		case BUTTON_ESCAPE: return("BUTTON_ESCAPE"); break;

		case BUTTON_SPACEBAR: return("BUTTON_SPACEBAR"); break;
		case BUTTON_PAGE_UP: return("BUTTON_PAGE_UP"); break;
		case BUTTON_PAGE_DOWN: return("BUTTON_PAGE_DOWN"); break;
		case BUTTON_END: return("BUTTON_END"); break;
		case BUTTON_HOME: return("BUTTON_HOME"); break;
		case BUTTON_LEFT: return("BUTTON_LEFT"); break;
		case BUTTON_UP: return("BUTTON_UP"); break;
		case BUTTON_RIGHT: return("BUTTON_RIGHT"); break;
		case BUTTON_DOWN: return("BUTTON_DOWN"); break;
		case BUTTON_SELECT: return("BUTTON_SELECT"); break;
		case BUTTON_PRINT: return("BUTTON_PRINT"); break;
		case BUTTON_EXECUTE: return("BUTTON_EXECUTE"); break;
		case BUTTON_PRINT_SCREEN: return("BUTTON_PRINT_SCREEN"); break;
		case BUTTON_INSERT: return("BUTTON_INSERT"); break;
		case BUTTON_DELETE: return("BUTTON_DELETE"); break;
		case BUTTON_HELP: return("BUTTON_HELP"); break;
		case BUTTON_0: return("BUTTON_0"); break;
		case BUTTON_1: return("BUTTON_1"); break;
		case BUTTON_2: return("BUTTON_2"); break;
		case BUTTON_3: return("BUTTON_3"); break;
		case BUTTON_4: return("BUTTON_4"); break;
		case BUTTON_5: return("BUTTON_5"); break;
		case BUTTON_6: return("BUTTON_6"); break;
		case BUTTON_7: return("BUTTON_7"); break;
		case BUTTON_8: return("BUTTON_8"); break;
		case BUTTON_9: return("BUTTON_9"); break;

		case BUTTON_A: return("BUTTON_A"); break;
		case BUTTON_B: return("BUTTON_B"); break;
		case BUTTON_C: return("BUTTON_C"); break;
		case BUTTON_D: return("BUTTON_D"); break;
		case BUTTON_E: return("BUTTON_E"); break;
		case BUTTON_F: return("BUTTON_F"); break;
		case BUTTON_G: return("BUTTON_G"); break;
		case BUTTON_H: return("BUTTON_H"); break;
		case BUTTON_I: return("BUTTON_I"); break;
		case BUTTON_J: return("BUTTON_J"); break;
		case BUTTON_K: return("BUTTON_K"); break;
		case BUTTON_L: return("BUTTON_L"); break;
		case BUTTON_M: return("BUTTON_M"); break;
		case BUTTON_N: return("BUTTON_N"); break;
		case BUTTON_O: return("BUTTON_O"); break;
		case BUTTON_P: return("BUTTON_P"); break;
		case BUTTON_Q: return("BUTTON_Q"); break;
		case BUTTON_R: return("BUTTON_R"); break;
		case BUTTON_S: return("BUTTON_S"); break;
		case BUTTON_T: return("BUTTON_T"); break;
		case BUTTON_U: return("BUTTON_U"); break;
		case BUTTON_V: return("BUTTON_V"); break;
		case BUTTON_W: return("BUTTON_W"); break;
		case BUTTON_X: return("BUTTON_X"); break;
		case BUTTON_Y: return("BUTTON_Y"); break;
		case BUTTON_Z: return("BUTTON_Z"); break;
		case BUTTON_WIN_L: return("BUTTON_WIN_L"); break;
		case BUTTON_WIN_R: return("BUTTON_WIN_R"); break;
		case BUTTON_APP: return("BUTTON_APP"); break;

		case BUTTON_SLEEP: return("BUTTON_SLEEP"); break;
		case BUTTON_NUMPAD_0: return("BUTTON_NUMPAD_0"); break; 
		case BUTTON_NUMPAD_1: return("BUTTON_NUMPAD_1"); break;
		case BUTTON_NUMPAD_2: return("BUTTON_NUMPAD_2"); break;
		case BUTTON_NUMPAD_3: return("BUTTON_NUMPAD_3"); break;
		case BUTTON_NUMPAD_4: return("BUTTON_NUMPAD_4"); break;
		case BUTTON_NUMPAD_5: return("BUTTON_NUMPAD_5"); break;
		case BUTTON_NUMPAD_6: return("BUTTON_NUMPAD_6"); break;
		case BUTTON_NUMPAD_7: return("BUTTON_NUMPAD_7"); break;
		case BUTTON_NUMPAD_8: return("BUTTON_NUMPAD_8"); break;
		case BUTTON_NUMPAD_9: return("BUTTON_NUMPAD_9"); break;
		case BUTTON_MULTIPLY: return("BUTTON_MULTIPLY"); break;
		case BUTTON_ADD: return("BUTTON_ADD"); break;
		case BUTTON_SEPARATOR: return("BUTTON_SEPARATOR"); break;
		case BUTTON_SUBTRACT: return("BUTTON_SUBTRACT"); break;
		case BUTTON_DECIMAL: return("BUTTON_DECIMAL"); break;
		case BUTTON_DIVIDE: return("BUTTON_DIVIDE"); break;

		case BUTTON_F1: return("BUTTON_F1"); break;
		case BUTTON_F2: return("BUTTON_F2"); break;
		case BUTTON_F3: return("BUTTON_F3"); break;
		case BUTTON_F4: return("BUTTON_F4"); break;
		case BUTTON_F5: return("BUTTON_F5"); break;
		case BUTTON_F6: return("BUTTON_F6"); break;
		case BUTTON_F7: return("BUTTON_F7"); break;
		case BUTTON_F8: return("BUTTON_F8"); break;
		case BUTTON_F9: return("BUTTON_F9"); break;
		case BUTTON_F10: return("BUTTON_F10"); break;
		case BUTTON_F11: return("BUTTON_F11"); break;
		case BUTTON_F12: return("BUTTON_F12"); break;
		case BUTTON_F13: return("BUTTON_F13"); break;
		case BUTTON_F14: return("BUTTON_F14"); break;
		case BUTTON_F15: return("BUTTON_F15"); break;
		case BUTTON_F16: return("BUTTON_F16"); break;
		case BUTTON_F17: return("BUTTON_F17"); break;
		case BUTTON_F18: return("BUTTON_F18"); break;
		case BUTTON_F19: return("BUTTON_F19"); break;
		case BUTTON_F20: return("BUTTON_F20"); break;
		case BUTTON_F21: return("BUTTON_F21"); break;
		case BUTTON_F22: return("BUTTON_F22"); break;
		case BUTTON_F23: return("BUTTON_F23"); break;
		case BUTTON_F24: return("BUTTON_F24"); break;

		case BUTTON_NUM_LOCK: return("BUTTON_NUM_LOCK"); break;
		case BUTTON_SCROLL_LOCK: return("BUTTON_SCROLL_LOCK"); break;

		case BUTTON_SHIFT_L: return("BUTTON_SHIFT_L"); break;
		case BUTTON_SHIFT_R: return("BUTTON_SHIFT_R"); break;
		case BUTTON_CTRL_L: return("BUTTON_CTRL_L"); break;
		case BUTTON_CTRL_R: return("BUTTON_CTRL_R"); break;
		case BUTTON_ALT_L: return("BUTTON_ALT_L"); break;
		case BUTTON_ALT_R: return("BUTTON_ALT_R"); break;

		case BUTTON_BROWSER_BACK: return("BUTTON_BROWSER_BACK"); break;
		case BUTTON_BROWSER_FORWARD: return("BUTTON_BROWSER_FORWARD"); break;
		case BUTTON_BROWSER_REFRESH: return("BUTTON_BROWSER_REFRESH"); break;
		case BUTTON_BROWSER_SEARCH: return("BUTTON_BROWSER_SEARCH"); break;
		case BUTTON_BROWSER_FAVORITES: return("BUTTON_BROWSER_FAVORITES"); break;
		case BUTTON_BROWSER_HOME: return("BUTTON_BROWSER_HOME"); break;
		case BUTTON_VOLUME_MUTE: return("BUTTON_VOLUME_MUTE"); break;
		case BUTTON_VOLUME_DOWN: return("BUTTON_VOLUME_DOWN"); break;
		case BUTTON_VOLUME_UP: return("BUTTON_VOLUME_UP"); break;
		case BUTTON_MEDIA_NEXT_TRACK: return("BUTTON_MEDIA_NEXT_TRACK"); break;
		case BUTTON_MEDIA_PREV_TRACK: return("BUTTON_MEDIA_PREV_TRACK"); break;
		case BUTTON_MEDIA_STOP: return("BUTTON_MEDIA_STOP"); break;
		case BUTTON_MEDIA_PLAY_PAUSE: return("BUTTON_MEDIA_PLAY_PAUSE"); break;

		case BUTTON_OEM_1: return("BUTTON_OEM_1"); break;
		case BUTTON_OEM_PLUS: return("BUTTON_OEM_PLUS"); break;
		case BUTTON_OEM_COMMA: return("BUTTON_OEM_COMMA"); break;
		case BUTTON_OEM_MINUS: return("BUTTON_OEM_MINUS"); break;
		case BUTTON_OEM_PERIOD: return("BUTTON_OEM_PERIOD"); break;
		case BUTTON_OEM_2: return("BUTTON_OEM_2"); break;
		case BUTTON_OEM_3: return("BUTTON_OEM_3"); break;

		case BUTTON_OEM_4: return("BUTTON_OEM_4"); break;
		case BUTTON_OEM_5: return("BUTTON_OEM_5"); break;
		case BUTTON_OEM_6: return("BUTTON_OEM_6"); break;
		case BUTTON_OEM_7: return("BUTTON_OEM_7"); break;
		case BUTTON_OEM_8: return("BUTTON_OEM_8"); break;

		case BUTTON_OEM_102: return("BUTTON_OEM_102"); break;

		case BUTTON_OEM_CLEAR: return("BUTTON_OEM_CLEAR"); break;
	}

	return "unknown";
}