#ifndef DECRYPTION
#define DECRYPTION

#include "Computer.h"
#include "Button.h"
#include "RadioButton.h"
#include "TextField.h"

struct Decryption : public ScreenElement
{
	Computer* parent;
	vector<ScreenElement*> children;
	Decryption(float _x1, float _y1, float _x2, float _y2, Computer* _parent, Application _application) : ScreenElement(_x1, _y1, _x2, _y2, "", _application), parent(_parent) {}
	void draw(TextureManager* texture_manager);
	int screen_number = 0;
	int frames_since_open = 0;
	Button* legal_button_ptr;
	RadioButton* legal_radio_ptr;
	TextField* required_text_field = NULL;
	void animate();
	void remove_children();
	int stick_counter = 0;
	int stick_frames = 0;
};

#endif