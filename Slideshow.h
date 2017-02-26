#ifndef SLIDESHOW
#define SLIDESHOW

#include "Computer.h"

struct Slideshow : public ScreenElement
{
	vector<string> slides;
	ScreenElement* circle;
	int frames = 0;
	bool animating = false;
	int offset = 0;
	int target_offset = 0;
	int cur_slide = 0;
	Slideshow(float _x1, float _y1, float _x2, float _y2, string _name, Application _application, vector<string> _slides, ScreenElement* _circle) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), slides(_slides), circle(_circle) {}
	void animate()
	{
		if (!animating)
		{
			frames++;
			if (frames % 200 == 0)
			{
				animating = true;
				if (cur_slide == slides.size() - 1)
					cur_slide = 0;

				else cur_slide++;
				//target_offset = cur_slide * (x2 - x1);
				animate_to(cur_slide);
			}
		}

		if (animating)
		{
			if (target_offset >= offset)
			{
				offset += 50;
				if (offset > target_offset)
				{
					offset = target_offset;
					animating = false;
				}
			}

			else if (target_offset < offset)
			{
				offset -= 50;
				if (offset < target_offset)
				{
					offset = target_offset;
					animating = false;
				}
			}
		}
	}

	void animate_to(int slide)
	{
		frames = 0;
		cur_slide = slide;
		target_offset = cur_slide * (x2 - x1);
		circle->x1 = 582 + slide * 84;
		circle->x2 = circle->x1 + 32;
		animating = true;
	}

	void draw(TextureManager* texture_manager)
	{
		glEnable(GL_STENCIL_TEST);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
		glStencilFunc(GL_NEVER, 1, 0xFF);
		glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
		glDisable(GL_TEXTURE_2D);
		glStencilMask(0xFF);
		glColor4f(1.0, 0.0, 0.0, 1.0);
		glRectf(scalex(x1), scaley(y1), scalex(x2), scaley(y2));
		glColor4f(1.0, 1.0, 1.0, 1.0);
		glEnable(GL_TEXTURE_2D);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glStencilMask(0x00);
		glStencilFunc(GL_EQUAL, 1, 0xFF);
		for (int i = 0; i < slides.size(); ++i)
		{
			int nx1 = i * (x2 - x1) - offset + x1;
			ScreenElement elem(nx1, y1, nx1 + (x2 - x1), y2, slides[i], application);
			elem.draw(texture_manager);
		}

		glStencilMask(~0);
		glClear(GL_STENCIL_BUFFER_BIT);
		glDisable(GL_STENCIL_TEST);
	}
};

#endif