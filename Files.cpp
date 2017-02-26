#include "Files.h"
#include "fonts.h"

//TODO limits on number of files you can create???
void Files::draw(TextureManager* texture_manager)
{
	ScreenElement::draw(texture_manager);
	//ScreenElement folder(770.0, 684.0, 820.0, 734.0, "icon_folder.png");
	ScreenElement folder(x1 + 10, y2 - 60, x1 + 60, y2 - 10, "icon_folder.png");
	//ScreenElement lock(800.0, 684.0, 820.0, 704.0, "lock.png");
	ScreenElement lock(x1 + 30, y2 - 70, x1 + 70, y2 - 30, "lock.png");
	auto list_of_files = parent->file_system[current_folder];
	for (int i = 0; i < list_of_files.size(); ++i)
	{
		ScreenElement file(x1 + 10 + 100.0*(i + 1), y2 - 60, x1 + 60 + 100.0*(i + 1), y2 - 10, "fileicon.png");
		file.draw(texture_manager);
		draw_string(texture_manager, 32, list_of_files[i].name, x1 + 10 + 100.0*(i + 1), y2 - 80);
	}

	if (draw_blue)
		glColor4f(0.0, 0.0, 1.0, 1.0);

	folder.draw(texture_manager);
	if (folder_index == folder_names.size() - 1)
		lock.draw(texture_manager);

	draw_string(texture_manager, 32, folder_names[folder_index], x1 + 10, y2 - 80);
	current_folder = folder_names[folder_index];
	glColor4f(1.0, 1.0, 1.0, 1.0);
}

void Files::mouse_clicked(int button, int state, int x, int y)
{
	if (state == GLUT_UP)
		return;

	else if (x1 + 10 <= x && x <= x1 + 60 && y2 - 60 <= y && y <= y2 - 10)
	{
		int time_clicked = glutGet(GLUT_ELAPSED_TIME);
		if (time_last_clicked < 0)
		{
			time_last_clicked = time_clicked;
			draw_blue = true;
			return;
		}

		else if (time_clicked - time_last_clicked < 750)
		{
			draw_blue = false;
			time_last_clicked = -1;
			if (folder_index == folder_names.size() - 1)
			{
				parent->tried_to_view_encrypted_folder = true;
				parent->start_application(INFO_BOX, "Cannot open folder.  Folder is encrypted.");
				return;
			}

			folder_index++;
			current_folder = folder_names[folder_index];
			text_field_ptr->text = get_cur_dir_name(folder_index);
		}

		else
		{
			time_last_clicked = time_clicked;
			draw_blue = true;
			return;
		}
	}

	else
		draw_blue = false;

	/*auto list_of_files = parent->file_system[current_folder];
	for (int i = 0; i < list_of_files.size(); ++i)
	{
		if (x1 + 10 + 100.0*(i + 1) <= x && x <= x1 + 60 + 100.0*(i + 1) && y2 - 60 <= y && y <= y2 - 10)
		{
			int time_clicked = glutGet(GLUT_ELAPSED_TIME);
			if (time_last_clicked < 0)
			{
				time_last_clicked = time_clicked;
				draw_blue = true;
				blue_file = i;
				return;
			}

			else if (time_clicked - time_last_clicked < 750)
			{

			}
		}
	}*/
}

void Files::write_image_file(GLubyte* image, int xsize, int ysize)
{
	File new_file;
	new_file.name = name_field_ptr->text;
	new_file.image = new GLubyte[xsize * ysize * 4]; //TODO PLEASE CLEAN THIS UP
	new_file.xsize = xsize;
	new_file.ysize = ysize;
	memcpy(new_file.image, image, xsize * ysize * 4);
	parent->file_system[current_folder].push_back(new_file);
}

void Files::load_image_file(Canvas* canvas)
{
	auto list_of_files = parent->file_system[current_folder];
	for (int i = 0; i < list_of_files.size(); ++i)
	{
		if (list_of_files[i].name == name_field_ptr->text)
		{
			canvas->resize(canvas->x1, canvas->y2 - list_of_files[i].ysize, canvas->x1 + list_of_files[i].xsize, canvas->y2);
			memcpy(canvas->image, list_of_files[i].image, list_of_files[i].xsize * list_of_files[i].ysize * 4);
			return;
		}
	}
}

//TODO likely need to tag all images with size when we can resize the canvas
void Files::upload_image_file()
{
	auto list_of_files = parent->file_system[current_folder];
	for (int i = 0; i < list_of_files.size(); ++i)
	{
		if (list_of_files[i].name == text_field_ptr->text)
		{
			parent->uploaded_image = list_of_files[i];
			parent->uploaded_image.image = new GLubyte[list_of_files[i].xsize * list_of_files[i].ysize * 4]; //TODO PLESE CLEAN THIS UP
			memcpy(parent->uploaded_image.image, list_of_files[i].image, list_of_files[i].xsize * list_of_files[i].ysize * 4); //so editing local file doesn't change upload
			return;
		}
	}
}

string Files::get_cur_dir_name(int index)
{
	if (index == 0)
		return "/";

	string name = "";
	for (int i = 0; i < index; ++i)
		name += "/" + folder_names[i];

	return name;
}