#pragma once

enum class Mode {
	PAL, NTSC
};


namespace settings {
	const int PAL_SCREEN_HEIGHT = 312;
	const int NTSC_SCREEN_HEIGHT = 263;
	const int PAL_SCREEN_WIDTH = 504;
	const int NTSC_SCREEN_WIDTH = 520;
	const int PAL_VISIBLE_WIDTH = 384;
	const int PAL_VISIBLE_HEIGHT = 272;
	extern int width;
	extern int height;
	extern int visible_width;
	extern int visible_height;
	extern int window_width;
	extern int window_height;
	extern Mode mode;

};