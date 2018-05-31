#ifndef FILES_HEADER
#define FILES_HEADER

#include <vector>
#include <string>
#include "Computer.h"
#include "TextField.h"
#include "Canvas.h"
#include "Button.h"

using std::vector;
using std::string;

struct Files : public ScreenElement
{
	Computer* parent;
	TextField* text_field_ptr;
	TextField* name_field_ptr;
	vector<ScreenElement*> children;
	function<void(string)> file_open_function;
	Files(float _x1, float _y1, float _x2, float _y2, string _name, Computer* _parent, Application _application) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), parent(_parent)
	{
		file_open_function = [](string file_name){};
		folder_index = 0;
		auto text_field = make_unique<TextField>(x1 + 10, y2 + 4 + 22, x1 + 10 + 347, y2 + 4 + 30 + 22, "systemtextfield.png", parent, application);
		text_field->text = "/";
		text_field_ptr = text_field.get();
		parent->to_be_added.push_back(std::move(text_field));
		auto go_button = make_unique<Button>(x2 - 8 - 27 - 20, y2 + 9 + 22, x2 - 8 - 27, y2 + 29 + 22, "go.png", application, [this]()
		{
			int index = 0;
			for (index = 0; index < parent->folder_names.size(); ++index)
			{
				if (this->text_field_ptr->text == parent->get_cur_dir_name(index))
					break;
			}

			if (index == parent->folder_names.size())
			{
				this->parent->start_application(INFO_BOX, "Directory does not exist.");
				return;
			}

			if (index == parent->folder_names.size() - 1)
			{
				if (!parent->tried_to_view_encrypted_folder)
					parent->email_counter = 1;

				parent->tried_to_view_encrypted_folder = true;
				parent->start_application(INFO_BOX, "", false, false, 100);
				parent->restore_application = FILES;
				return;
			}

			this->enter_directory(index);
		}, [](){}, "", "golight.png", "godark.png", "", false, true);
		auto go_button_ptr = go_button.get();
		text_field_ptr->on_enter_function = [go_button_ptr]() {go_button_ptr->release_function();};
		//go_button->translate(1100 - go_button->x2, 0);
		parent->to_be_added.push_back(std::move(go_button));
		auto up_button = make_unique<Button>(x2 - 8 - 27, y2 + 9 + 22, x2 - 8, y2 + 29 + 22, "up.png", application, [this]()
		{
			if (this->folder_index == 0)
				return;

			this->enter_directory(this->folder_index - 1);
		}, [](){}, "", "uplight.png", "updark.png", "", false, true);
		//up_button->translate(1140 - up_button->x2, 0);
		parent->to_be_added.push_back(std::move(up_button));
	}

	void draw(TextureManager* texture_manager);
	bool write_image_file(GLubyte* image, int xsize, int ysize);
	void remove_children();
	void enter_directory(int new_index);
	int folder_index;
	string current_folder;
	bool draw_blue = false;
	int time_last_clicked = -1;
	int blue_file = -1;
	void create_test_files();
	void delete_file(string file_name);
};

#endif