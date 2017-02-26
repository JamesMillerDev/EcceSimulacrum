#include "Console.h"
#include "keymappings.h"

void Console::draw(TextureManager* texture_manager)
{
	ScreenElement::draw(texture_manager);
	parent->draw_messages(570.0, -1.0 * (glutGet(GLUT_WINDOW_HEIGHT) - 744.0), 0.5);
}

void Console::print_to_grid(string text)
{
	vector<char> row;
	for (int i = 0; i < text.size(); ++i)
	{
		row.push_back(text[i]);
		if (row.size() == grid_width || i == text.size() - 1)
		{
			parent->char_grid.push_back(row);
			row.clear();
			if (parent->char_grid.size() > grid_height)
				parent->char_grid.erase(parent->char_grid.begin());
		}
	}
}

void Console::give_focus()
{
	ScreenElement::give_focus();
	int size = parent->char_grid.size();
	if (parent->char_grid[size - 1].size() < grid_width)
	{
		parent->char_grid[size - 1].pop_back();
		parent->char_grid[size - 1].push_back('_');
	}

	else
		print_to_grid("_");
}

void Console::take_focus()
{
	ScreenElement::take_focus();
	int size = parent->char_grid.size();
	parent->char_grid[size - 1].pop_back();
	parent->char_grid[size - 1].push_back(' ');
}

void Console::press_key(unsigned char key)
{
	if (key == '\r')
	{
		string input;
		parent->char_grid[parent->char_grid.size() - 1].pop_back();
		for (int i = parent->char_grid.size() - 1 - input_row; i < parent->char_grid.size(); ++i)
		{
			vector<char> row = parent->char_grid[i];
			for (int j = 0; j < row.size(); ++j)
				input += row[j];
		}

		input.erase(0, 2);

		if (input == "help")
		{
			print_to_grid("Here are some commands to get you started:");
			print_to_grid("ps - list all running processes");
			print_to_grid("kill [id] - kills process with id [id]");
		}

		else if (input == "ps")
		{
			if (!parent->recovered_browser)
				print_to_grid("Browser - 9713");
			
			print_to_grid("Explorer - 7193");
			print_to_grid("svcspool1 - 6713");
			print_to_grid("svcspool2 - 8174");
			print_to_grid("sysrecv - 7195");
		}

		else if (input == "kill 9713")
		{
			parent->recovered_browser = true;
		}

		else
			print_to_grid("Command " + input + " not recognized.");
		
		print_to_grid("/>_");
		input_row = 0;
	}

	else if (key == KEY_BACKSPACE)
	{
		int size = parent->char_grid.size();
		if (input_row == 0 && parent->char_grid[size - 1].size() == 3)
			return;

		if (parent->char_grid[size - 1].size() == 1)
		{
			parent->char_grid.pop_back();
			input_row--;
		}

		else
		{
			parent->char_grid[size - 1].pop_back();
			parent->char_grid[size - 1].pop_back();
			parent->char_grid[size - 1].push_back('_');
		}
	}

	else if (32 <= key && key <= 126)
	{
		int size = parent->char_grid.size();
		if (parent->char_grid[size - 1].size() == grid_width)
		{
			parent->char_grid[size - 1][grid_width - 1] = key;
			print_to_grid("_");
			input_row++;
		}

		else
		{
			parent->char_grid[size - 1].pop_back();
			parent->char_grid[size - 1].push_back(key);
			parent->char_grid[size - 1].push_back('_');
		}
	}
}