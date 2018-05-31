#ifndef BLOCKS
#define BLOCKS

#include "Computer.h"

struct Blocks : public ScreenElement
{
	Blocks(float _x1, float _y1, float _x2, float _y2, Application _application) : ScreenElement(_x1, _y1, _x2, _y2, "black.png", _application) {}
	void draw(TextureManager* texture_manager)
	{
		ScreenElement::draw(texture_manager);
		for (int i = x1; i < x2; i += 76)
		{
			for (int j = y1; j < y2 / 2.0; j += 76)
			{
				int choice = rand() % 5;
				string to_draw;
				if (choice == 0)
					to_draw = "redblock.png";

				if (choice == 1)
					to_draw = "blueblock.png";

				if (choice == 2)
					to_draw = "greenblock.png";

				if (choice == 3)
					to_draw = "yellowblock.png";

				if (choice == 4)
					to_draw = "seafoamblock.png";

				ScreenElement block(i, j, i + 76, j + 76, to_draw);
				block.draw(texture_manager);
				if (j >= 76 * 3)
				{
					if (rand() % 2 == 0)
						break;
				}
			}
		}
	}
};

#endif