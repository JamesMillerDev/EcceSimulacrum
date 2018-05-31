#ifndef SHIMMERING_TEXT
#define SHIMMERING_TEXT

#include "Computer.h"

struct ShimmeringText : public ScreenElement
{
	vector<ScreenElement> shimmers;
	int frames;
	float alpha;
	ShimmeringText(float _x1, float _y1, float _x2, float _y2, string _name, Application _application) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application)
	{
		frames = 0;
		alpha = 0.0;
		int cur_x = 0;
		for (;;)
		{
			if (cur_x >= 1920)
				break;

			shimmers.push_back(ScreenElement(cur_x, y1, cur_x + 800, y1 + 200, "shimmer.png"));
			cur_x += 800;
		}
	}

	void animate()
	{
		frames++;
		if (frames <= 60)
			alpha += 0.01;

		if (frames >= 60 * 5 && alpha > 0.0)
			alpha -= 0.01;

		for (int i = 0; i < shimmers.size(); ++i)
			shimmers[i].translate(40, 0);

		if (shimmers.back().x1 >= 1920)
			shimmers.pop_back();

		if (shimmers[0].x1 >= 0)
			shimmers.insert(shimmers.begin(), ScreenElement(shimmers[0].x1 - 800, y1, shimmers[0].x1, y1 + 200, "shimmer.png"));
	}

	void draw(TextureManager* texture_manager)
	{
		glEnable(GL_STENCIL_TEST);
		glEnable(GL_ALPHA_TEST);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_FALSE);
		glStencilFunc(GL_NEVER, 1, 0xFF);
		glAlphaFunc(GL_GREATER, 0.0);
		glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
		glStencilMask(0xFF);
		ScreenElement::draw(texture_manager);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glStencilMask(0x00);
		glStencilFunc(GL_EQUAL, 1, 0xFF);
		glColor4f(1.0, 1.0, 1.0, alpha);
		for (int i = 0; i < shimmers.size(); ++i)
			shimmers[i].draw(texture_manager);

		glStencilMask(~0);
		glClear(GL_STENCIL_BUFFER_BIT);
		glDisable(GL_STENCIL_TEST);
		glDisable(GL_ALPHA_TEST);
		glColor4f(1.0, 1.0, 1.0, 1.0);
	}
};

#endif