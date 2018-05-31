#include "Files.h"
#include "fonts.h"
#include "binaryio.h"

//TODO limits on number of files you can create??? and scrolling?
//TODO should create the buttons in draw or something... Files won't automatically refresh when new files are added
//TODO some folder names are randomly blurry
void Files::draw(TextureManager* texture_manager)
{
	ScreenElement::draw(texture_manager);
}

void Files::enter_directory(int new_index)
{
	this->folder_index = new_index;
	this->current_folder = ((new_index == 0) ? "/" : parent->folder_names[this->folder_index - 1]);
	this->text_field_ptr->text = parent->get_cur_dir_name(this->folder_index);
	text_field_ptr->go_to_end();

	remove_children();
	if (current_folder != "q")
	{
		auto folder_button = make_unique<Button>(x1 + 15, y2 - 60 + 20, x1 + 65, y2 - 10 + 20, (current_folder == "imeanit" && !parent->decryption_broken) ? "lockedfolder.png" : "|icon_folder.png", application, []() {}, []() {});
		folder_button->parent = parent;
		auto folder_button_ptr = folder_button.get();
		folder_button_ptr->footer = parent->folder_names[folder_index];
		folder_button_ptr->on_click_function = [folder_button_ptr]() { folder_button_ptr->draw_blue = true; };
		folder_button_ptr->double_click_function = [this]()
		{
			if (this->current_folder == "imeanit" && !parent->decryption_broken)
			{
				if (!parent->tried_to_view_encrypted_folder)
					parent->email_counter = 1;

				parent->tried_to_view_encrypted_folder = true;
				parent->start_application(INFO_BOX, "", false, false, 100);
				parent->restore_application = FILES;
			}

			else this->enter_directory(this->folder_index + 1);
		};
		folder_button_ptr->handle_double_click = true;
		folder_button_ptr->info_box_id = info_box_id;
		children.push_back(folder_button_ptr);
		parent->to_be_added.push_back(std::move(folder_button));
	}
	auto list_of_files = parent->get_folder_index(current_folder);
	int xcoord = 1;
	int ycoord = 0;
	for (int i = 0; i < list_of_files.size(); ++i)
	{
		int type = list_of_files[i].type;
		string texture_name;
		if (type == 0)
			texture_name = "fileicon.png";

		else if (type == 1)
			texture_name = "|icon_console.png";

		else if (type == 2)
			texture_name = "codeicon.png";

		auto file_button = make_unique<Button>(x1 + 15 + 100.0 * xcoord, y2 - 60 - (100 * ycoord) + 20, x1 + 65 + 100.0 * xcoord, y2 - 10 - (100 * ycoord) + 20, texture_name, application, []() {}, []() {});
		file_button->parent = parent;
		auto file_button_ptr = file_button.get();
		file_button_ptr->footer = list_of_files[i].name;
		file_button_ptr->on_click_function = [file_button_ptr]() { file_button_ptr->draw_blue = true; };
		string file_name = list_of_files[i].name;
		file_button_ptr->right_click_function = [this, file_name, file_button_ptr]()
		{
			parent->start_application(INFO_BOX, "Delete file?", false, false, 0, 0, true);
			parent->translate_application(parent->to_be_added, parent->prev_mouse_x, parent->prev_mouse_y, INFO_BOX, true);
			parent->dialogue_confirm_function = [this, file_name]() {this->delete_file(file_name);};
			file_button_ptr->draw_blue = true;
		};
		file_button_ptr->double_click_function = [this, file_name, type]() { if (type == 0) { this->file_open_function(file_name); } };
		file_button_ptr->handle_double_click = true;
		file_button_ptr->info_box_id = info_box_id;
		children.push_back(file_button_ptr);
		parent->to_be_added.push_back(std::move(file_button));
		xcoord++;
		if (xcoord >= 7)
		{
			ycoord++;
			xcoord = 0;
		}
	}
}

void Files::remove_children()
{
	for (int i = 0; i < parent->screen_elements.size(); ++i)
	{
		for (int j = 0; j < children.size(); ++j)
		{
			if (children[j] == parent->screen_elements[i].get())
				parent->screen_elements[i]->marked_for_deletion = true;
		}
	}

	children.clear();
}

bool Files::write_image_file(GLubyte* image, int xsize, int ysize)
{
	if (parent->get_folder_size(current_folder) >= 20)
	{
		parent->start_application(INFO_BOX, "Folder has reached capacity.");
		return false;
	}
	
	File file;
	file.xsize = xsize;
	file.ysize = ysize;
	file.type = 0;
	file.name = name_field_ptr->text;
	file.image = image;
	bool success = parent->write_file_to_folder(file, current_folder);
	if (!success)
	{
		parent->start_application(INFO_BOX, "Could not write file. Make sure the file name is valid and there is sufficient disk space.");
		return false;
	}

	return true;
}

void Files::create_test_files()
{
	string old_current = current_folder;
	string old_text = name_field_ptr->text;
	auto image = new GLubyte[1700 * 700 * 4];
	for (int i = 0; i < 1700 * 700 * 4; ++i)
		image[i] = 255;

	for (int i = 0; i < parent->folder_names.size(); ++i)
	{
		current_folder = parent->folder_names[i];
		for (int i = 1; i <= 20; ++i)
		{
			name_field_ptr->text = string(i, 'a');
			write_image_file(image, 1700, 700);
		}
	}

	current_folder = old_current;
	name_field_ptr->text = old_text;
}

void Files::delete_file(string file_name)
{
	parent->delete_file(file_name, current_folder);
	enter_directory(folder_index);
}