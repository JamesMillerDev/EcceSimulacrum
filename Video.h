#ifndef VIDEO
#define VIDEO

#include <Windows.h>
#include <Vfw.h>
#include "Computer.h"

struct Video : public ScreenElement
{
	Computer* parent;
	Video(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, Computer* _parent);
	void draw(TextureManager* texture_manager);
	void animate();
	~Video();
	void press_key(unsigned char key);
	void release_key(unsigned char key);
	GLuint texture;
	int current_frame = 1000;//1; //TODO 0?
	PAVISTREAM stream;
	PGETFRAME frame_object;
	unsigned char* data = 0;
	int width, height;
	map<unsigned char, bool> is_key_pressed;
};

#endif