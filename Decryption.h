#ifndef DECRYPTION
#define DECRYPTION

#include "Computer.h"
#include "Button.h"
#include "RadioButton.h"
#include "TextField.h"
#include "SplashScreen.h"

struct Decryption : public ScreenElement
{
	Computer* parent;
	vector<ScreenElement*> children;
	Decryption(float _x1, float _y1, float _x2, float _y2, Computer* _parent, Application _application) : ScreenElement(_x1, _y1, _x2, _y2, "", _application), parent(_parent)
	{
		auto splash_screen = make_unique<SplashScreen>(0, 0, 500, 200, "splashscreen.png", application);
		splash_screen->center();
		children.push_back(splash_screen.get());
		parent->to_be_added.push_back(std::move(splash_screen));
	}

	void draw(TextureManager* texture_manager);
	int screen_number = 0;
	int frames_since_open = 0;
	Button* legal_button_ptr;
	TextField* required_text_field = NULL;
	void animate();
	void remove_children();
	int stick_counter = 0;
	int stick_frames = 0;
};

#endif