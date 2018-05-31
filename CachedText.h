#ifndef CACHED_TEXT
#define CACHED_TEXT

#include "Computer.h"
#include "fonts.h"

struct CachedText : public ScreenElement
{
	string text;
	GLuint texture;
	bool drawn_before;
	CachedText(float _x1, float _y1, float _x2, float _y2, string name, string _text, Application application) : ScreenElement(_x1, _y1, _x2, _y2, name, application), text(_text)
	{
		drawn_before = false;
	}

	void center_on(int yval, TextureManager* texture_manager)
	{
		int height = draw_string(texture_manager, 32, text, 0, 0, false, false, 0, false, false, false, true).back();
		translate(0, yval - (height / 2) - y1);
	}

	void draw(TextureManager* texture_manager)
	{
		if (!drawn_before)
		{
			drawn_before = true;
			y2 = y1 + 30;
			x2 = x1 + draw_string(texture_manager, 32, text, 0, 0, false).back();
			GLint old_fbo;
			GLuint framebuffer;
			glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &old_fbo);
			glGenFramebuffers(1, &framebuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
			glGenTextures(1, &texture);
			glBindTexture(GL_TEXTURE_2D, texture);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x2 - x1, y2 - y1, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
			ScreenElement background(0, 0, x2, y2 - y1, "surveycolor.png");
			background.draw(texture_manager);
			draw_string(texture_manager, 32, text, 0, 5, true, false, 0, true);
			glBindFramebuffer(GL_FRAMEBUFFER, old_fbo);
			glDeleteFramebuffers(1, &framebuffer);
		}

		glBindTexture(GL_TEXTURE_2D, texture);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2f(scalex((int)x1), scaley((int)y1-5));
		glTexCoord2f(0.0, 1.0); glVertex2f(scalex((int)x1), scaley((int)y2-5));
		glTexCoord2f(1.0, 1.0); glVertex2f(scalex((int)x2), scaley((int)y2-5));
		glTexCoord2f(1.0, 0.0); glVertex2f(scalex((int)x2), scaley((int)y1-5));
		glEnd();
	}

	~CachedText()
	{
		glDeleteTextures(1, &texture);
	}
};

#endif