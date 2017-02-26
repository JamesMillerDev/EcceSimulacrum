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
	Files(float _x1, float _y1, float _x2, float _y2, string _name, Computer* _parent, Application _application) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), parent(_parent)
	{
		folder_names = { "users", "xj5555585", "documents", "pics", "oldstuff", "random", "stage1", "stopit", "systemdata", "goaway", "imeanit", "q" };
		folder_index = 0;
		auto text_field = make_unique<TextField>(770.0 + 10 - 300, 744 - 60 + 10, 1000, 744 - 10, "textfield.png", parent, application);
		text_field->text = "/";
		text_field_ptr = text_field.get();
		parent->to_be_added.push_back(std::move(text_field));
		auto go_button = make_unique<Button>(1010, 744 - 60 + 10, 1060, 744 - 10, "go.png", application, [this]()
		{
			int index = 0;
			for (index = 0; index < this->folder_names.size(); ++index)
			{
				if (this->text_field_ptr->text == this->get_cur_dir_name(index))
					break;
			}

			if (index == folder_names.size())
			{
				this->parent->start_application(INFO_BOX, "Directory does not exist.");
				return;
			}

			if (index == folder_names.size() - 1)
			{
				this->parent->tried_to_view_encrypted_folder = true;
				this->parent->start_application(INFO_BOX, "Cannot open folder.  Folder is encrypted.");
				return;
			}

			this->folder_index = index;
			this->current_folder = this->folder_names[index];
			this->text_field_ptr->text = this->get_cur_dir_name(index);
		}, [](){}, "", "golight.png", "godark.png");
		auto go_button_ptr = go_button.get();
		text_field_ptr->on_enter_function = [go_button_ptr]() {go_button_ptr->release_function();};
		parent->to_be_added.push_back(std::move(go_button));
		auto up_button = make_unique<Button>(1070, 744 - 60 + 10, 1120, 744 - 10, "arrow.png", application, [this]()
		{
			if (this->folder_index == 0)
				return;

			this->folder_index--;
			this->current_folder = this->folder_names[this->folder_index];
			this->text_field_ptr->text = this->get_cur_dir_name(this->folder_index);
		}, [](){}, "", "genlight.png", "gendark.png");
		parent->to_be_added.push_back(std::move(up_button));
	}

	void mouse_clicked(int button, int state, int x, int y);
	void draw(TextureManager* texture_manager);
	void write_image_file(GLubyte* image, int xsize, int ysize);
	void load_image_file(Canvas* canvas);
	void upload_image_file();
	string get_cur_dir_name(int index);
	vector<string> folder_names;
	int folder_index;
	string current_folder;
	bool draw_blue = false;
	int time_last_clicked = -1;
	int blue_file = -1;
};

#endif