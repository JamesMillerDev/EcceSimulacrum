#include "Video.h"
#include "cMovie.h"

Video::Video(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, Computer* _parent) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), parent(_parent)
{
	glGenTextures(1, &texture);
	AVIFileInit();
	AVIStreamOpenFromFile(&stream, "video.avi", streamtypeVIDEO, 0, OF_READ, NULL);
	AVISTREAMINFO stream_info;
	AVIStreamInfo(stream, &stream_info, sizeof(stream_info));
	width = stream_info.rcFrame.right - stream_info.rcFrame.left;
	height = stream_info.rcFrame.bottom - stream_info.rcFrame.top;
	frame_object = AVIStreamGetFrameOpen(stream, PBITMAPINFOHEADER(AVIGETFRAMEF_BESTDISPLAYFMT));
	/*alListener3f(AL_POSITION, 0, 0, 0);
	ALfloat orientation[] = { 0, 0, 1, 0, 1, 0 };
	alListenerfv(AL_ORIENTATION, orientation);
	alSource3f(parent->sound_sources[0], AL_POSITION, 0, 0, 0.5);
	alSourcePlay(parent->sound_sources[0]);*/
}

void Video::press_key(unsigned char key)
{
	//if (!((48 <= key && key <= 57) || (97 <= key && key <= 122)))
	//	return;

	is_key_pressed[key] = true;
}

void Video::release_key(unsigned char key)
{
	is_key_pressed[key] = false;
}

void Video::draw(TextureManager* texture_manager)
{
	LPBITMAPINFOHEADER frame_data = (LPBITMAPINFOHEADER)AVIStreamGetFrame(frame_object, current_frame);
	data = (unsigned char*)frame_data + frame_data->biSize + frame_data->biClrUsed * sizeof(RGBQUAD);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	GLenum glerr = glGetError();
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2f(scalex(x1), scaley(y1));
		glTexCoord2f(0.0, 1.0); glVertex2f(scalex(x1), scaley(y2));
		glTexCoord2f(1.0, 1.0); glVertex2f(scalex(x2), scaley(y2));
		glTexCoord2f(1.0, 0.0); glVertex2f(scalex(x2), scaley(y1));
	glEnd();

	int num_keys_pressed = 0;
	for (unsigned char key = 48; key <= 57; ++key)
	{
		if (is_key_pressed[key])
			num_keys_pressed++;
	}

	for (unsigned char key = 97; key <= 122; ++key)
	{
		if (is_key_pressed[key])
			num_keys_pressed++;
	}

	if (num_keys_pressed == 0)
		return;

	ScreenElement bar(400, 100, 900, 200, "white.png");
	ScreenElement progress(400, 100, 400.0 + ((float)num_keys_pressed / 36.0)*500.0, 200, "green.png");
	bar.draw(texture_manager);
	progress.draw(texture_manager);
}

void Video::animate()
{
	current_frame++;
	if (current_frame >= 1500)
	{
		parent->close_application(VIDEO_APP);
		parent->process_control_changes();
	}
}

Video::~Video()
{
	AVIStreamGetFrameClose(frame_object);
	AVIStreamRelease(stream);
	AVIFileExit();
}