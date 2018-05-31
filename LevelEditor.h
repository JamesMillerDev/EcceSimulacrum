#ifndef LEVEL_EDITOR
#define LEVEL_EDITOR

#include "ObjModel.h"
#include "Computer.h"

struct LevelEditor : public ScreenElement
{
	int xoffset = 0;
	int yoffset = 0;
	bool never_clicked = true;
	Computer* parent;
	list<vector<string>> text_contents;
	ScreenElement active_piece;
	LevelEditor(int _x1, int _y1, int _x2, int _y2, string _name, Computer* _parent, Application _application) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), parent(_parent),
		active_piece(0, 0, 100, 200, "tree.png"), never_clicked(true)
	{
		x2 = parent->texture_manager->get_width(name);
		y2 = parent->texture_manager->get_height(name);
		ifstream file_stream(name + ".txt");
		string line;
		while (getline(file_stream, line))
		{
			vector<string> components = real_split(line, ' ');
			text_contents.push_back(components);
		}
	}

	void draw(TextureManager* texture_manager);
	void mouse_moved(int x, int y);
	void mouse_clicked(int button, int state, int x, int y);
	void press_key(unsigned char key);
	void write_text_and_overlay();
};

#endif