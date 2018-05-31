#ifndef SPLASH_SCREEN
#define SPLASH_SCREEN

#include "Computer.h"
#include "fonts.h"

struct SplashScreen : public ScreenElement
{
	vector<string> messages;
	int index = 0;
	int frames = 0;
	SplashScreen(float _x1, float _y1, float _x2, float _y2, string _name, Application _application) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), index(0), frames(0)
	{
		messages = { "Loading application...", "Loading DLLs...", "Initializing...", "Establishing window parameters...", "Starting application..." };
	}

	void animate()
	{
		if (frames % 3 == 0)
			index++;

		if (index >= messages.size())
			index = 0;

		frames++;
	}

	void draw(TextureManager* texture_manager)
	{
		ScreenElement::draw(texture_manager);
		draw_string(texture_manager, 32, messages[index], x1 + 10, y1 + 10);
	}
};

#endif