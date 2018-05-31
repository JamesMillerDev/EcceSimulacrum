#include "LevelEditor.h"
#include "keymappings.h"

void LevelEditor::mouse_moved(int x, int y)
{
	active_piece.translate(x - active_piece.x1, y - active_piece.y1);
}

void LevelEditor::press_key(unsigned char key)
{
	if (key == KEY_LEFT)
		xoffset += 1920;

	if (key == KEY_RIGHT)
		xoffset -= 1920;

	if (key == KEY_UP)
		yoffset -= 1080;

	if (key == KEY_DOWN)
		yoffset += 1080;

	if (key == 's')
		write_text_and_overlay();
}

void LevelEditor::mouse_clicked(int button, int state, int x, int y)
{
	if (never_clicked)
	{
		never_clicked = false;
		return;
	}
	
	if (state != GLUT_DOWN)
		return;

	if (button == GLUT_LEFT_BUTTON)
	{
		vector<string> new_line{ active_piece.name, to_string(active_piece.x1 - xoffset), to_string(active_piece.y1 - yoffset), to_string(active_piece.x2 - xoffset), to_string(active_piece.y2 - yoffset) };
		text_contents.push_back(new_line);
	}

	if (button == GLUT_RIGHT_BUTTON)
	{
		for (auto it = --text_contents.end(); it != text_contents.begin(); --it)
		{
			int ex1 = atoi((*it)[1].c_str()) + xoffset;
			int ey1 = atoi((*it)[2].c_str()) + yoffset;
			int ex2 = atoi((*it)[3].c_str()) + xoffset;
			int ey2 = atoi((*it)[4].c_str()) + yoffset;
			if (ex1 <= x && x <= ex2 && ey1 <= y && y <= ey2)
			{
				if (!((*it)[0] == "bg" || (*it)[0] == "sp" || (*it)[0] == "warp" || (*it)[0] == "ladder"))
				{
					text_contents.erase(it);
					break;
				}
			}
		}
	}
}

void LevelEditor::draw(TextureManager* texture_manager)
{
	translate(xoffset, yoffset);
	ScreenElement::draw(texture_manager);
	translate(-xoffset, -yoffset);
	for (auto it = text_contents.begin(); it != text_contents.end(); ++it)
	{
		if ((*it)[0] == "bg" || (*it)[0] == "sp" || (*it)[0] == "warp" || (*it)[0] == "ladder")
			continue;

		ScreenElement elem = ScreenElement(atoi((*it)[1].c_str()), atoi((*it)[2].c_str()), atoi((*it)[3].c_str()), atoi((*it)[4].c_str()), (*it)[0]);
		elem.translate(xoffset, yoffset);
		elem.draw(texture_manager);
	}

	glColor4f(1.0, 1.0, 1.0, 0.5);
	active_piece.draw(texture_manager);
	glColor4f(1.0, 1.0, 1.0, 1.0);
}

void LevelEditor::write_text_and_overlay()
{
	std::ofstream out_stream;
	out_stream.open(name + ".txt", std::ofstream::out | std::ofstream::trunc);
	for (auto it = text_contents.begin(); it != text_contents.end(); ++it)
	{
		string line = "";
		for (int i = 0; i < (*it).size(); ++i)
			line += (*it)[i] + " ";

		out_stream << line + "\n";
	}

	GLubyte* buffer = (GLubyte*)malloc(sizeof(GLubyte) * x2 * y2 * 4);
	memset(buffer, 0, sizeof(GLubyte) * x2 * y2 * 4);
	for (auto it = text_contents.begin(); it != text_contents.end(); ++it)
	{
		if ((*it)[0] == "bg" || (*it)[0] == "sp" || (*it)[0] == "warp" || (*it)[0] == "ladder")
			continue;

		int elem_width = parent->texture_manager->get_width((*it)[0]);
		int elem_height = parent->texture_manager->get_height((*it)[0]);
		GLubyte* elem_data = parent->texture_manager->get_pixel_data((*it)[0]);
		for (int i = 0; i < elem_width; ++i)
		{
			for (int j = 0; j < elem_height; ++j)
			{
				int elem_start = j * (elem_width) * 4 + i * 4;
				int buffer_x = atoi((*it)[1].c_str());
				int buffer_y = atoi((*it)[2].c_str());
				//int buffer_start = (j + buffer_y) * x2 * 4 + (i + buffer_x) * 4;
				int step1 = (j + buffer_y) * x2 * 4;
				int step2 = (i + buffer_x) * 4;
				int buffer_start = step1 + step2;
				if (i + buffer_x < 0 || i + buffer_x >= x2 || j + buffer_y < 0 || j + buffer_y >= y2)
					continue;

				if (elem_data[elem_start + 3] == 0)
					continue;

				buffer[buffer_start] = elem_data[elem_start];
				buffer[buffer_start + 1] = elem_data[elem_start + 1];
				buffer[buffer_start + 2] = elem_data[elem_start + 2];
				buffer[buffer_start + 3] = elem_data[elem_start + 3];
			}
		}
	}

	parent->texture_manager->save_image(real_split(name, '.')[0] + "overlay.png", buffer, x2, y2);
	free(buffer);
}