#include "Console.h"
#include "ObjModel.h"
#include "fonts.h"
#include "keymappings.h"

//TODO cd with relative pathnames

void Console::draw(TextureManager* texture_manager)
{
	ScreenElement::draw(texture_manager);
	//parent->draw_messages(570.0, -1.0 * (glutGet(GLUT_WINDOW_HEIGHT) - 744.0), 0.39);
	change_font("LiberationMono-Bold.ttf");
	for (int i = 0; i < parent->char_grid.size(); ++i)
	{
		string row = "";
		for (int j = 0; j < parent->char_grid[i].size(); ++j)
			row += parent->char_grid[i][j];

		if (has_focus && draw_cursor && i == parent->char_grid.size() - 1)
		{
			if (row.size() == grid_width)
				draw_string(texture_manager, 32, "_", x1 + 1, y2 - ((i + 2) * ((404.0 - 390.0) + 4.0)), true, true, 0, false, false, false);

			else row += "_";
		}

		draw_string(texture_manager, 32, row, x1 + 1, y2 - ((i + 1) * ((404.0 - 390.0) + 4.0)), true, true, 0, false, false, false);
	}

	change_font("Lato-Regular.ttf");
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

void Console::animate()
{
	if (has_focus)
	{
		frames++;
		if (frames % 20 == 0)
			draw_cursor = !draw_cursor;
	}
}

void Console::give_focus()
{
	ScreenElement::give_focus();
}

void Console::take_focus()
{
	ScreenElement::take_focus();
	draw_cursor = true;
	frames = 1;
}

//TODO if you just save an image file with the right name, it can act as the lesson3 or whatever exe or code right?
void Console::press_key(unsigned char key)
{
	frames = 1;
	draw_cursor = true;
	if (key == '\r')
	{
		string input;
		for (int i = parent->char_grid.size() - 1 - input_row; i < parent->char_grid.size(); ++i)
		{
			vector<char> row = parent->char_grid[i];
			for (int j = 0; j < row.size(); ++j)
				input += row[j];
		}

		input.erase(0, current_directory.length() + 1);

		auto tokens = real_split(input, ' ');
		if (tokens[0] == "help" && tokens.size() == 1)
		{
			print_to_grid(" ");
			print_to_grid(" Here are some commands to get you started:");
			print_to_grid("   ps - list all running processes and their id numbers");
			print_to_grid("   kill [id] - kills process with id [id]");
			print_to_grid("   cd [directory] - changes the current directory to [directory]");
			print_to_grid("   run [program] [option1] ... [optionN] - runs the program [program] from the current    directory with arguments [option1] through [optionN]");
			print_to_grid(" There are no other commands.");
			print_to_grid(" ");
		}

		else if (tokens[0] == "ps" && tokens.size() == 1)
		{
			print_to_grid(" ");
			print_to_grid("       name|   id");
			print_to_grid("------------------");
			if (!parent->recovered_browser)
				print_to_grid("   Internet| 9713");
			
			print_to_grid("  svcspool1| 6713");
			print_to_grid("  svcspool2| 8174");
			print_to_grid("    sysrecv| 7195");
			print_to_grid("Current security settings do not allow for information on other running applications.");
			print_to_grid(" ");
		}

		else if (tokens[0] == "kill")
		{
			if (tokens.size() == 2)
			{
				if (!parent->recovered_browser && tokens[1] == "9713")
					parent->recovered_browser = true;

				else print_to_grid("Cannot kill id '" + tokens[1] + "'");
			}

			else print_to_grid("Command '" + input + "' not recognized.");
		}

		else if (tokens[0] == "cd")
		{
			if (tokens.size() == 2)
			{
				bool success = false;
				for (int i = 0; i < parent->folder_names.size(); ++i)
				{
					if (tokens[1] == parent->get_cur_dir_name(i))
					{
						success = true;
						auto folders = real_split(tokens[1], '/'); //TODO this has to be able to return more than 10 things...
						current_directory = folders[folders.size() - 1];
						if (tokens[1] == "/")
							current_directory = "/";
					}
				}

				if (!success)
				{
					for (int i = 0; i < parent->folder_names.size(); ++i)
					{
						if (i < parent->folder_names.size() - 1)
						{
							if ((parent->folder_names[i] == current_directory && parent->folder_names[i + 1] == tokens[1] || (i == 0 && parent->folder_names[i] == tokens[1])))
							{
								current_directory = tokens[1];
								success = true;
							}
						}
					}
				}

				if (!success)
					print_to_grid("Directory '" + tokens[1] + "' does not exist");
			}
		}

		else if (tokens[0] == "run")
		{
			if (tokens.size() != 2 && tokens.size() != 3)
				print_to_grid("Incorrect syntax");

			else
			{
				if (!parent->file_present(current_directory, tokens[1]))
					print_to_grid("Incorrect syntax");

				else
				{
					if (tokens[1] == "bcc")
					{
						if (tokens.size() != 3)
							print_to_grid("Incorrect syntax");

						else
						{
							if (!parent->file_present(current_directory, tokens[2]))
								print_to_grid("File '" + tokens[2] + "' not found");

							else
							{
								if (tokens[2] == "lesson1" || tokens[2] == "lesson2" || tokens[2] == "lesson3")
								{
									File program;
									program.name = tokens[2] + "exe";
									program.type = 1;
									parent->write_file_to_folder(program, current_directory);
								}

								else print_to_grid("Could not compile file '" + tokens[2] + "'");
							}
						}
					}

					else if (tokens[1] == "lesson1exe")
						print_to_grid("Lesson 1!");

					else if (tokens[1] == "lesson2exe")
						print_to_grid("Lesson 2!");

					else if (tokens[1] == "lesson3exe")
						parent->decryption_broken = true;
				}
			}
		}

		else
			print_to_grid("Command '" + input + "' not recognized.");
		
		print_to_grid(current_directory + ">");
		input_row = 0;
	}

	else if (key == KEY_BACKSPACE)
	{
		int size = parent->char_grid.size();
		if (input_row == 0 && parent->char_grid[size - 1].size() == current_directory.length() + 1)
			return;

		if (parent->char_grid[size - 1].size() == 0)
		{
			parent->char_grid.pop_back();
			parent->char_grid[size - 2].pop_back();
			input_row--;
		}

		else parent->char_grid[size - 1].pop_back();
	}

	else if (32 <= key && key <= 126)
	{
		int size = parent->char_grid.size();
		if (parent->char_grid[size - 1].size() == grid_width)
		{
			print_to_grid(string(1, key));
			input_row++;
		}

		else parent->char_grid[size - 1].push_back(key);
	}
}