#include "Video.h"
#include "cMovie.h"

Video::Video(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, Computer* _parent) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), parent(_parent)
{
	glGenTextures(1, &texture);
	AVIFileInit();
	AVIStreamOpenFromFile(&stream, "video3.avi", streamtypeVIDEO, 0, OF_READ, NULL);
	AVISTREAMINFO stream_info;
	AVIStreamInfo(stream, &stream_info, sizeof(stream_info));
	width = stream_info.rcFrame.right - stream_info.rcFrame.left;
	height = stream_info.rcFrame.bottom - stream_info.rcFrame.top;
	frame_object = AVIStreamGetFrameOpen(stream, PBITMAPINFOHEADER(AVIGETFRAMEF_BESTDISPLAYFMT));
}

void Video::draw(TextureManager* texture_manager)
{
	LPBITMAPINFOHEADER frame_data = (LPBITMAPINFOHEADER)AVIStreamGetFrame(frame_object, current_frame);
	data = (unsigned char*)frame_data + frame_data->biSize + frame_data->biClrUsed * sizeof(RGBQUAD);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	GLenum glerr = glGetError();
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2f(scalex(x1), scaley(y1));
		glTexCoord2f(0.0, 1.0); glVertex2f(scalex(x1), scaley(y2));
		glTexCoord2f(1.0, 1.0); glVertex2f(scalex(x2), scaley(y2));
		glTexCoord2f(1.0, 0.0); glVertex2f(scalex(x2), scaley(y1));
	glEnd();
}

void Video::animate()
{
	current_frame++;
	if (current_frame >= 250)
		current_frame = 1;
}

Video::~Video()
{
	AVIStreamGetFrameClose(frame_object);
	AVIStreamRelease(stream);
	AVIFileExit();
}