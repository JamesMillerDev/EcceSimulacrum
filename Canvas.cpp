#include <algorithm>
#include "Canvas.h"
#include "Button.h"
#include "Files.h"
#include "ColorPickerButton.h"
#include "InfoText.h"
#include "SizeSlider.h"

//TODO 250, 157, 204 hue off by 1?
//TODO fix line algorithm, sometimes stacks segments on top of each other
//TODO movable color picker (bonus feature)
//TODO release mode framerate problems with rectangle, circle large sizes round brush
//TODO framerate drops while dragging color slider
//TODO super bonus: smooth edges on circles
//TODO better cursors that change color
//TODO sometimes scroll wheel and size keys change move cursor to regular brush cursor while sitting still

int clamp_to(int lower, int upper, int value)
{
	if (value < lower)
		return lower;

	if (value > upper)
		return upper;

	return value;
}

Canvas::Canvas(float _x1, float _y1, float _x2, float _y2, string _name, Computer* _parent, Application _application) : ScreenElement(_x1, _y1, _x2, _y2, _name, _application), parent(_parent)
{
	increasing_size = false;
	decreasing_size = false;
	circle_brush = true;
	receive_out_of_bounds_clicks = true;
	x1 = floor(scalex(x1));
	x2 = floor(scalex(x2));
	y1 = floor(scaley(y1));
	y2 = floor(scaley(y2));
	no_scale = true;
	selection_active = false;
	dragging_selection = false;
	image = new GLubyte[(int)(x2 - x1) * (int)(y2 - y1) * 4];
	temp_image = new GLubyte[(int)(x2 - x1) * (int)(y2 - y1) * 4];
	backup_image = new GLubyte[(int)(x2 - x1) * (int)(y2 - y1) * 4];
	backup_width = x2 - x1;
	backup_height = y2 - y1;
	if (!parent)
		return;

	for (int i = 0; i < (x2 - x1) * (y2 - y1) * 4; ++i)
	{
		image[i] = 255;
		backup_image[i] = 255;
		temp_image[i] = 0;
	}

	auto color_picker_button = make_unique<ColorPickerButton>(20.0, 1004.0 + 6, 56.0, 1040.0 + 6, "buttonblank.png", PAINT, this);
	parent->to_be_added.push_back(std::move(color_picker_button));
	for (int i = 61; i < 186; i += 25)
	{
		auto palette_button = make_unique<ColorPickerButton>(i, 1020 + 6, i + 20, 1040 + 6, "buttonblank.png", PAINT, this, false);
		auto history_button = make_unique<ColorPickerButton>(i, 989 + 6, i + 20, 1009 + 6, "buttonblank.png", PAINT, this, false, true);
		if (i + 25 >= 186)
		{
			palette_button->label_text = "Palette";
			history_button->label_text = "History";
		}

		color_history.push_back(history_button.get());
		parent->to_be_added.push_back(std::move(palette_button));
		parent->to_be_added.push_back(std::move(history_button));
	}

	auto size_field = make_unique<TextField>(490 + 180 + 80 + 407 + 369, 1004 - 7 + 6, 530 + 180 + 80 + 407 + 369 + 8, 1040 - 7 + 6, "textfield.png", parent, PAINT);
	size_box = size_field.get();
	size_box->text = to_string(brush_size);
	set<unsigned char> allowed{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
	size_box->allowed_characters = allowed;
	size_box->take_focus_function = [this]()
	{
		int value = atoi(this->size_box->text.c_str());
		if (value == 0)
		{
			value = 1;
			this->size_box->reset();
			this->size_box->text = "1";
		}

		if (value > 100)
		{
			value = 100;
			this->size_box->reset();
			this->size_box->text = "100";
		}

		this->brush_size = value;
	};

	size_box->after_typing_function = [this]()
	{
		int value = atoi(this->size_box->text.c_str());
		if (value > 100)
			value = 100;

		this->brush_size = (value == 0) ? 1 : value;
	};

	parent->to_be_added.push_back(std::move(size_field));
	parent->to_be_added.push_back(make_unique<InfoText>(440 + 180 + 80 + 407 + 369, 1015-7 + 6, 480 + 180 + 80 + 407 + 369, 1015-7 + 6, "invisible.png", "Size:", PAINT));
	parent->to_be_added.push_back(make_unique<SizeSlider>(1486 - 5, 1080 - 66 - 5, 1516 + 5, 1080 - 55 + 5, "invisible.png", PAINT, true, this));
	auto alpha_field = make_unique<TextField>(600 + 180 + 110 + 407 + 369, 1004 - 7 + 6, 648 + 180 + 110 + 407 + 369, 1040-7 + 6, "textfield.png", parent, PAINT);
	parent->to_be_added.push_back(make_unique<InfoText>(525 + 180 + 110 + 407 + 369, 1015-7 + 6, 485 + 180 + 110 + 407 + 369, 1015-7 + 6, "invisible.png", "Opacity:", PAINT));
	parent->to_be_added.push_back(make_unique<SizeSlider>(1601 - 5, 1080 - 69 - 5, 1657 + 5, 1080 - 55 + 5, "invisible.png", PAINT, false, this));
	parent->to_be_added.push_back(make_unique<InfoText>(672 + 180 + 80 + 407 + 369, 1015-7 + 6, 672 + 180 + 80 + 407 + 369, 1015-7 + 6, "invisible.png", "%", PAINT));
	parent->to_be_added.push_back(make_unique<InfoText>(1393 + 369 + 43, 1015 - 7 + 6, 1393 + 369 + 43, 1015 - 7 + 6, "invisible.png", "Round Brush", PAINT));
	auto round_check_box = make_unique<Button>(1370 + 369 + 43, 1015 - 10 + 6, 1390 + 369 + 43, 1015 + 10 + 6, "checkbox.png", PAINT, [this]() {this->circle_brush = !this->circle_brush;}, []() {}, "", "checkboxlight.png", "checkboxdark.png");
	round_check_box->check_box = true;
	parent->to_be_added.push_back(std::move(round_check_box));
	alpha_box = alpha_field.get();
	alpha_box->text = to_string((int)((float)color_alpha / 255.0 * 100));
	alpha_box->allowed_characters = allowed;
	alpha_box->take_focus_function = [this]()
	{
		int value = atoi(this->alpha_box->text.c_str());
		if (value == 0)
		{
			value = 1;
			this->alpha_box->reset();
			this->alpha_box->text = "1";
		}

		if (value > 100)
		{
			value = 100;
			this->alpha_box->reset();
			this->alpha_box->text = "100";
		}

		this->color_alpha = ((float)value / 100.0) * 255.0;
	};

	alpha_box->after_typing_function = [this]()
	{
		int value = atoi(this->alpha_box->text.c_str());
		this->color_alpha = (value > 100) ? 255.0 : ((float)value / 100.0) * 255.0;
	};

	parent->to_be_added.push_back(std::move(alpha_field));
}

void Canvas::make_backup()
{
	did_backup = true;
	if (backup_width != x2 - x1 || backup_height != y2 - y1)
	{
		delete[] backup_image;
		backup_image = new GLubyte[(int)(x2 - x1) * (int)(y2 - y1) * 4];
		backup_width = x2 - x1;
		backup_height = y2 - y1;
	}

	memcpy(backup_image, image, (int)(x2 - x1) * (int)(y2 - y1) * 4 * sizeof(GLubyte));
}

void Canvas::undo()
{
	if (!did_backup)
		return;

	if (draw_selection_rect)
	{
		finalize_selection();
		undo();
		undo();
	}
	
	bool make_new_backup = false;
	GLubyte* new_backup = 0;
	if (backup_width != x2 - x1 || backup_height != y2 - y1)
	{
		new_backup = new GLubyte[(int)(x2 - x1) * (int)(y2 - y1) * 4];
		int new_backup_width = x2 - x1;
		int new_backup_height = y2 - y1;
		memcpy(new_backup, image, (int)(x2 - x1) * (int)(y2 - y1) * 4 * sizeof(GLubyte));
		resize(x1, y2 - backup_height, x1 + backup_width, y2, false);
		make_new_backup = true;
		backup_width = new_backup_width;
		backup_height = new_backup_height;
	}
	
	memcpy(temp_image, image, (int)(x2 - x1) * (int)(y2 - y1) * 4 * sizeof(GLubyte));
	memcpy(image, backup_image, (int)(x2 - x1) * (int)(y2 - y1) * 4 * sizeof(GLubyte));
	memcpy(backup_image, temp_image, (int)(x2 - x1) * (int)(y2 - y1) * 4 * sizeof(GLubyte));
	clear_temp();
	if (make_new_backup)
	{
		delete[] backup_image;
		backup_image = new_backup;
	}
}

void Canvas::push_color(bool use_color, GLubyte red, GLubyte green, GLubyte blue)
{
	for (int i = color_history.size() - 1; i >= 0; --i)
	{
		if (i == 0)
		{
			if (use_color)
			{
				color_history[i]->self_color_red = red;
				color_history[i]->self_color_green = green;
				color_history[i]->self_color_blue = blue;
			}
			
			else
			{
				color_history[i]->self_color_red = color_red;
				color_history[i]->self_color_green = color_green;
				color_history[i]->self_color_blue = color_blue;
			}
		}

		else
		{
			color_history[i]->self_color_red = color_history[i - 1]->self_color_red;
			color_history[i]->self_color_green = color_history[i - 1]->self_color_green;
			color_history[i]->self_color_blue = color_history[i - 1]->self_color_blue;
		}
	}
}

void Canvas::change_tool(string name)
{
	if (current_tool == SELECT)
		finalize_selection();

	if (name == "undo")
	{
		undo();
		return;
	}

	if (name == "help")
		return;

	if (name == "filledellipse")
		current_tool = FILLED_ELLIPSE;

	if (name == "eraser")
		current_tool = ERASER;

	if (name == "ellipse")
		current_tool = ELLIPSE;

	else if (name == "fill")
		current_tool = FILL;

	else if (name == "filledrectangle")
		current_tool = FILLED_RECTANGLE;

	else if (name == "line")
		current_tool = LINE;

	else if (name == "brush")
		current_tool = BRUSH;

	else if (name == "rectangle")
		current_tool = RECTANGLE;

	else if (name == "select")
		current_tool = SELECT;

	else if (name == "stickylines")
		current_tool = STICKY_LINES;

	else if (name == "colorpicker")
		current_tool = COLOR_PICKER;

	for (int i = 0; i < tools.size(); ++i)
	{
		if (tools[i]->note == name)
			tools[i]->dark = true;

		else tools[i]->dark = false;
	}
}

void Canvas::resize(int nx1, int ny1, int nx2, int ny2, bool backup)
{
	finalize_selection();
	if (backup)
		make_backup();

	GLubyte* new_image = new GLubyte[(int)(nx2 - nx1) * (int)(ny2 - ny1) * 4];
	for (int j = 0; j < ny2 - ny1; ++j)
	{
		for (int i = 0; i < nx2 - nx1; ++i)
		{
			int start = j * (nx2 - nx1) * 4 + i * 4;
			if (i < x2 - x1 && (j - ((ny2 - ny1) - (y2 - y1)) >= 0))
			{
				Vector4 rgb = get_color(i, j - ((ny2 - ny1) - (y2 - y1)));
				new_image[start] = rgb.x;
				new_image[start + 1] = rgb.y;
				new_image[start + 2] = rgb.z;
				new_image[start + 3] = 255;
			}

			else
				new_image[start] = new_image[start + 1] = new_image[start + 2] = new_image[start + 3] = 255;
		}
	}

	delete[] image;
	delete[] temp_image;
	temp_image = new GLubyte[(int)(nx2 - nx1) * (int)(ny2 - ny1) * 4];
	for (int i = 0; i < (int)(nx2 - nx1) * (int)(ny2 - ny1) * 4; ++i)
		temp_image[i] = 0;

	image = new_image;
	x1 = nx1;
	x2 = nx2;
	y1 = ny1;
	y2 = ny2;
}

Vector4 Canvas::get_color(int x, int y, GLubyte* pointer)
{
	if (pointer == NULL)
		pointer = image;
	
	if (x < 0 || x >= (x2 - x1) || y < 0 || y >= (y2 - y1))
		return Vector4(-1, -1, -1, -1);

	int start = y * (x2 - x1) * 4 + x * 4;
	return Vector4(pointer[start], pointer[start + 1], pointer[start + 2], pointer[start + 3]);
}

void Canvas::set_pixel(int x, int y, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLubyte* pointer)
{
	if (pointer == NULL)
		pointer = image;

	if (x < 0 || x >= x2 - x1 || y < 0 || y >= y2 - y1)
		return;
	
	int start = y * (x2 - x1) * 4 + x * 4;
	if (pointer == temp_image)
	{
		pointer[start] = r;
		pointer[start + 1] = g;
		pointer[start + 2] = b;
		pointer[start + 3] = a;
	}

	else
	{
		auto cur_color = get_color(x, y, pointer);
		pointer[start] = r*(a / 255.0) + cur_color.x*(1.0 - (a / 255.0));
		pointer[start + 1] = g*(a / 255.0) + cur_color.y*(1.0 - (a / 255.0));
		pointer[start + 2] = b*(a / 255.0) + cur_color.z*(1.0 - (a / 255.0));
		pointer[start + 3] = 255;
	}
}

void Canvas::brush_stroke(int x, int y, GLubyte* pointer)
{
	if (brush_size == 1)
	{
		set_pixel(x, y, color_red, color_green, color_blue, color_alpha, pointer);
		return;
	}
	
	if (circle_brush && brush_size > 3)
	{
		circle_brush = false;
		int old_size = brush_size;
		brush_size = 1;
		draw_ellipse(x, y, old_size, old_size, pointer, true);
		brush_size = old_size;
		circle_brush = true;
		return;
	}

	int old_size = brush_size;
	brush_size = 1;
	draw_rectangle(x - old_size, y - old_size, x + old_size, y + old_size, pointer, true);
	brush_size = old_size;
	return;
	/*switch (brush_size)
	{
	case 1:
		set_pixel(x, y, color_red, color_green, color_blue, color_alpha, pointer);
		break;

	default:
		int start_x, start_y;
		if (brush_size % 2 == 0)
		{
			start_x = x - (brush_size / 2 - 1);
			start_y = y - (brush_size / 2 - 1);
		}

		else
		{
			start_x = x - brush_size / 2;
			start_y = y - brush_size / 2;
		}

		int end_y = y + brush_size / 2;
		int end_x = x + brush_size / 2;
		if (start_x < 0)
			start_x = 0;

		if (start_y < 0)
			start_y = 0;

		if (end_x >= x2 - x1)
			end_x = x2 - x1 - 1;

		if (end_y >= y2 - y1)
			end_y = y2 - y1 - 1;
		
		for (int j = start_y; j <= end_y; ++j)
		{
			for (int i = start_x; i <= end_x; ++i)
			{
				int start = j * (x2 - x1) * 4 + i * 4;
				pointer[start] = color_red;
				pointer[start + 1] = color_green;
				pointer[start + 2] = color_blue;
				pointer[start + 3] = color_alpha;
			}
		}
	}*/
}

void Canvas::clear_temp()
{
	memset(temp_image, 0, (x2 - x1) * (y2 - y1) * 4 * sizeof(GLubyte));
}

void Canvas::change_size(int delta)
{
	int num = atoi(size_box->text.c_str());
	brush_size = num + delta;
	if (brush_size < 1)
		brush_size = 1;

	if (brush_size > 100)
		brush_size = 100;

	size_box->reset();
	size_box->text = to_string(brush_size);
	if (!dragging_selection)
		mouse_moved(prev_moved_x, prev_moved_y);
}

void Canvas::animate()
{
	if (decreasing_size)
		change_size(-1);

	if (increasing_size)
		change_size(1);

	if (decreasing_red)
	{
		if (color_red > 0)
			color_red -= 5;

		if (!dragging_selection)
			mouse_moved(prev_moved_x, prev_moved_y);
	}
	
	if (increasing_red)
	{
		if (color_red < 255)
			color_red += 5;

		if (!dragging_selection)
			mouse_moved(prev_moved_x, prev_moved_y);
	}

	if (decreasing_green)
	{
		if (color_green > 0)
			color_green -= 5;

		if (!dragging_selection)
			mouse_moved(prev_moved_x, prev_moved_y);
	}

	if (increasing_green)
	{
		if (color_green < 255)
			color_green += 5;

		if (!dragging_selection)
			mouse_moved(prev_moved_x, prev_moved_y);
	}

	if (decreasing_blue)
	{
		if (color_blue > 0)
			color_blue -= 5;

		if (!dragging_selection)
			mouse_moved(prev_moved_x, prev_moved_y);
	}

	if (increasing_blue)
	{
		if (color_blue < 255)
			color_blue += 5;

		if (!dragging_selection)
			mouse_moved(prev_moved_x, prev_moved_y);
	}
}

void Canvas::press_key(unsigned char key)
{
	//if (current_tool == SELECT)
	//	write_temp_to_canvas(); maybe important when we have real tool switching
	
	/*clear_temp();
	temp_offset_x = 0;
	temp_offset_y = 0;
	selection_active = false;
	dragging_selection = false; //TODO do this on every tool change*/
	if (key == 'w')
		//brush_size = 1;
		increasing_green = true;

	if (key == 'p')
	{
		draw_line(50, 100, 50, 200, image);
		color_blue = 255;
		draw_line(50, 100, 50, 100, image);
	}
	
	if (key == 'e')
		increasing_blue = true;

	if (key == 'r')
	{
		color_red = 255;
		color_green = 255;
		color_blue = 255;
	}

	if (key == 't')
	{
		color_red = 0;
		color_green = 0;
		color_blue = 0;
	}

	if (key == 'y')
		current_tool = LINE;

	if (key == 'u')
		current_tool = BRUSH;

	if (key == 'i')
		current_tool = FILL;

	if (key == 'o')
		current_tool = ELLIPSE;

	if (key == 'p')
		current_tool = SELECT;

	if (key == 'a')
	{
		/*int num = atoi(size_box->text.c_str());
		if (num > 1)
		{
			brush_size = num - 1;
			size_box->reset();
			size_box->text = to_string(num - 1);
		}*/
		decreasing_size = true;
	}

	if (key == 'd')
	{
		/*int num = atoi(size_box->text.c_str());
		if (num < 100)
		{
			brush_size = num + 1;
			size_box->reset();
			size_box->text = to_string(num + 1);
		}*/
		increasing_size = true;
	}

	if (key == 'b')
		current_tool = COLOR_PICKER;

	if (key == 's')
	{
		save_file();
	}

	if (key == 'n')
	{
		load_file();
	}

	if (key == 'z')
		decreasing_red = true;

	if (key == 'c')
		decreasing_blue = true;

	if (key == 'q')
		increasing_red = true;

	if (key == 'x')
		decreasing_green = true;
}

void Canvas::release_key(unsigned char key)
{
	if (key == 'a')
		decreasing_size = false;

	if (key == 'd')
		increasing_size = false;

	if (key == 'z')
		decreasing_red = false;

	if (key == 'q')
		increasing_red = false;

	if (key == 'w')
		increasing_green = false;

	if (key == 'x')
		decreasing_green = false;

	if (key == 'c')
		decreasing_blue = false;

	if (key == 'e')
		increasing_blue = false;
}

void Canvas::save_file()
{
	parent->start_application(FILES, "", true);
	parent->restore_application = PAINT;
	auto files = make_unique<Files>(770.0 - 300, 404.0 - 60, 1150.0, 744.0 - 60, "white.png", parent, INFO_BOX);
	auto files_ptr = files.get();
	auto button = make_unique<Button>(955.0, /*409.0*/394 - 120 + 8 + 10, 1145.0, /*459.0*/394 - 120 + 8 + 60 - 10, "ok.png", INFO_BOX,
		[this, files_ptr]()
	{
		if (files_ptr->name_field_ptr->text == "")
		{
			this->parent->start_application(INFO_BOX, "Must specify a file name."); //TODO revisit this when fix bug about one info box closing all info boxes
			return;
		}

		if (files_ptr->name_field_ptr->text.size() > 30)
		{
			this->parent->start_application(INFO_BOX, "File name must be 30 characters or less.");
			return;
		}

		bool success = files_ptr->write_image_file(image, (int)(x2 - x1), (int)(y2 - y1));
		if (success)
			this->parent->close_application(INFO_BOX);
	}, []() {});
	auto name_box = make_unique<TextField>(945 - 400, 394 - 120 + 8 + 10, 945, 394 - 120 + 8 + 60 - 10, "systemtextfield.png", parent, INFO_BOX);
	files_ptr->name_field_ptr = name_box.get();
	parent->to_be_added.insert(parent->to_be_added.end() - 1, std::move(files));
	files_ptr->enter_directory(0);
	parent->to_be_added.push_back(std::move(name_box));
	parent->to_be_added.push_back(std::move(button));
	parent->to_be_added.push_back(make_unique<InfoText>(770 - 300, 394 - 120 + 8 + 10 + 15, 770 - 300, 394 - 120 + 8 + 10 + 15, "invisible.png", "Name:"));
	parent->tag_info_box();
	/*for (int i = 0; i < 20; ++i)
	{
		files_ptr->name_field_ptr->text = to_string(i);
		files_ptr->write_image_file(NULL, 0, 0);
	}*/

	//files_ptr->create_test_files();
}

void Canvas::load_file()
{
	parent->start_application(FILES, "", true);
	parent->restore_application = PAINT;
	auto files = make_unique<Files>(770.0 - 300, 404.0 - 60, 1150.0, 744.0 - 60, "white.png", parent, INFO_BOX);
	auto files_ptr = files.get();
	auto button = make_unique<Button>(955.0, /*409.0*/394 - 120 + 8 + 10, 1145.0, /*459.0*/394 - 120 + 8 + 60 - 10, "ok.png", INFO_BOX,
		[this, files_ptr]()
	{
		if (files_ptr->name_field_ptr->text == "")
		{
			this->parent->start_application(INFO_BOX, "Must specify a file name."); //TODO revisit this when fix bug about one info box closing all info boxes
			return;
		}

		this->load_image_file(files_ptr->name_field_ptr->text, files_ptr->current_folder);
		this->parent->close_application(INFO_BOX);
	}, []() {});
	auto name_box = make_unique<TextField>(945 - 400, 394 - 120 + 8 + 10, 945, 394 - 120 + 8 + 60 - 10, "systemtextfield.png", parent, INFO_BOX);
	files_ptr->name_field_ptr = name_box.get();
	parent->to_be_added.insert(parent->to_be_added.end() - 1, std::move(files));
	files_ptr->enter_directory(0);
	files_ptr->file_open_function = [this, files_ptr](string file_name) { this->load_image_file(file_name, files_ptr->current_folder); this->parent->close_application(INFO_BOX); };
	parent->to_be_added.push_back(std::move(name_box));
	parent->to_be_added.push_back(std::move(button));
	parent->to_be_added.push_back(make_unique<InfoText>(770 - 300, 394 - 120 + 8 + 10 + 15, 770 - 300, 394 - 120 + 8 + 10 + 15, "invisible.png", "Name:"));
	parent->tag_info_box();
}

void Canvas::load_image_file(string file_name, string folder_name)
{
	File file = parent->read_image_file(file_name, folder_name);
	if (file.xsize != 0)
	{
		resize(x1, y2 - file.ysize, x1 + file.xsize, y2);
		memcpy(image, file.image, file.xsize * file.ysize * 4);
	}

	free(file.image);
}

void Canvas::mouse_over(int x, int y)
{
	mouse_on = true;
	//parent->give_focus_to(this); TODO I think this was for size/opacity control?
	if ((current_tool == BRUSH || current_tool == ERASER || current_tool == LINE || current_tool == STICKY_LINES || current_tool == RECTANGLE || current_tool == FILLED_RECTANGLE || current_tool == ELLIPSE || current_tool == FILLED_ELLIPSE || current_tool == SELECT) && !being_resized)
	{
		parent->set_cursor("brushcursor.png");
		parent->maintain_cursor = true;
		if (current_tool == SELECT && dragging_selection)
			parent->set_cursor("cursormove.png");
	}

	if (current_tool == FILL && !being_resized)
	{
		parent->set_cursor("cursorfill.png");
		parent->maintain_cursor = true;
	}

	if (current_tool == COLOR_PICKER && !being_resized)
	{
		parent->set_cursor("cursorpicker.png");
		parent->maintain_cursor = true;
	}
}

void Canvas::mouse_off()
{
	if (mouse_on)
	{
		mouse_on = false;
		if (!parent->cursor_lock)
		{
			parent->set_cursor("cursor.png");
			parent->maintain_cursor = false;
		}

		if ((current_tool == BRUSH || current_tool == ERASER || current_tool == LINE || current_tool == STICKY_LINES) && !mouse_held)
			clear_temp();
	}
}

vector<pair<int, int>> Canvas::fill_from(int x, int y, GLubyte* pointer)
{
	if (pointer == NULL)
		pointer = image;

	make_backup();
	vector<pair<int, int>> active_points;
	vector<pair<int, int>> filled_points;
	Vector4 target_color = get_color(x, y, pointer);
	if (target_color.x == color_red && target_color.y == color_green && target_color.z == color_blue)
		return vector<pair<int, int>>();

	active_points.push_back(pair<int, int>(x, y));
	while (!active_points.empty())
	{
		auto active_point = active_points[active_points.size() - 1];
		int cur_x = active_point.first, cur_y = active_point.second;
		active_points.pop_back();
		int direction = 1;
		bool waiting_above = false, waiting_below = false;
		for (int i = 0; i < 2; ++i)
		{
			bool done = false;
			while (!done)
			{
				auto cur_color = get_color(cur_x, cur_y, pointer);
				if (cur_color.x == target_color.x && cur_color.y == target_color.y && cur_color.z == target_color.z)
				{
					set_pixel(cur_x, cur_y, color_red, color_green, color_blue, color_alpha, pointer);
					filled_points.push_back(pair<int, int>(cur_x, cur_y));
				}

				auto color_above = get_color(cur_x, cur_y + 1, pointer);
				auto color_below = get_color(cur_x, cur_y - 1, pointer);
				if (color_above.x == target_color.x && color_above.y == target_color.y && color_above.z == target_color.z)
				{
					if (!waiting_above)
					{
						if (!(cur_x == active_point.first && cur_y == active_point.second && direction == -1))
							active_points.push_back(pair<int, int>(cur_x, cur_y + 1));

						waiting_above = true;
					}
				}

				else if (waiting_above)
					waiting_above = false;

				if (color_below.x == target_color.x && color_below.y == target_color.y && color_below.z == target_color.z)
				{
					if (!waiting_below)
					{
						if (!(cur_x == active_point.first && cur_y == active_point.second && direction == -1))
							active_points.push_back(pair<int, int>(cur_x, cur_y - 1));

						waiting_below = true;
					}
				}

				else if (waiting_below)
					waiting_below = false;

				cur_x += direction;
				auto next_color = get_color(cur_x, cur_y, pointer);
				if (!(next_color.x == target_color.x && next_color.y == target_color.y && next_color.z == target_color.z))
					done = true;
			}

			cur_x = active_point.first; 
			direction = -1;
		}
	}

	return filled_points;
}

bool Canvas::temp_non_zero()
{
	for (int i = 0; i < (x2 - x1) * (y2 - y1) * 4; ++i)
	{
		if (temp_image[i] != 0)
			return true;
	}

	return false;
}

void Canvas::write_temp_to_canvas(bool backup)
{
	if (backup && temp_non_zero()) //TODO is this a good assumption?
		make_backup();
	
	for (int i = 0; i < x2 - x1; ++i)
	{
		for (int j = 0; j < y2 - y1; ++j)
		{
			Vector4 color = get_color(i, j, temp_image);
			if (color.q != 0)
				set_pixel(i + temp_offset_x, j + temp_offset_y, color.x, color.y, color.z, color.q, image);
		}
	}
}

void Canvas::change_opacity(int delta)
{
	int num = atoi(alpha_box->text.c_str());
	if (delta > 0)
	{
		if (num < 100)
		{
			if (100 - num < delta)
				num = 100;

			else num += delta;
			alpha_box->text = to_string(num);
			color_alpha = ((float)num / 100.0) * 255.0;
		}
	}

	else if (delta < 0)
	{
		if (num >= abs(delta))
			num += delta;

		else num = 0;
		alpha_box->text = to_string(num);
		color_alpha = ((float)num / 100.0) * 255.0;
	}

	if (!dragging_selection)
		mouse_moved(prev_moved_x, prev_moved_y);
}

void Canvas::mouse_clicked(int button, int state, int x, int y)
{
	//TODO currently assuming every tool must click down first before releasing
	/*if (button != GLUT_LEFT && button != 3 && button != 4)
		return;*/

	if (button == 1)
		return;
	
	if (button == GLUT_RIGHT_BUTTON)
	{
		if (current_tool == SELECT)
		{
			selection_rect.x1 -= temp_offset_x;
			selection_rect.x2 -= temp_offset_x;
			selection_rect.y1 -= temp_offset_y;
			selection_rect.y2 -= temp_offset_y;
			temp_offset_x = temp_offset_y = 0;
			finalize_selection();
		}

		clear_temp();
		mouse_held = false;
		selection_active = false;
		dragging_selection = false;
		mouse_moved(prev_moved_x, prev_moved_y);
		canvas_write_blocker = true;
		return;
	}

	if (button == 3)
	{
		//TODO went with 5 here because scroll wheel events seem to register twice, does that happen on every machine?
		change_opacity(5);
		return;
	}

	if (button == 4)
	{
		change_opacity(-5);
		return;
	}
	
	if (state == GLUT_UP)
	{
		if (current_tool == SELECT && mouse_held)
		{
			mouse_held = false;
			selection_active = true;
			int actual_x, actual_y;
			actual_x = x < x1 ? x1 : (x > x2 ? x2 : x);
			actual_y = y < y1 ? y1 : (y > y2 ? y2 : y);
			//draw_dotted_rectangle(prev_mouse_x, prev_mouse_y, actual_x - x1, actual_y - y1, temp_image); //TODO problem if the mouse is moving too fast?
			if (dragging_selection)
			{
				dragging_selection = false;
				draw_selection_rect = true;
				return;
			}

			dragging_selection = false;
			int first_y = prev_mouse_y;
			int second_y = y - y1;
			int first_x = prev_mouse_x;
			int second_x = x - x1;
			if (second_y < first_y)
				std::swap(first_y, second_y);

			if (second_x < first_x)
				std::swap(first_x, second_x);

			if (first_x != second_x && first_y != second_y)
				make_backup();

			for (int i = first_x; i < second_x; ++i)
			{
				for (int j = first_y; j < second_y; ++j)
				{
					Vector4 color = get_color(i, j, image);
					set_pixel(i, j, color.x, color.y, color.z, color.q, temp_image);
					set_pixel(i, j, 255, 255, 255, 255, image);
				}
			}

			return;
		}

		if (current_tool == COLOR_PICKER)
		{
			mouse_held = false;
			Vector4 color = get_color(x - x1, y - y1, image);
			if (color_red == color.x && color_green == color.y && color_blue == color.z)
				return;

			push_color();
			color_red = color.x;
			color_green = color.y;
			color_blue = color.z;
			return;
		}

		mouse_held = false;
		if (current_tool != FILL && current_tool != SELECT)
		{
			if (!canvas_write_blocker)
			{
				write_temp_to_canvas();
				clear_temp();
			}

			else canvas_write_blocker = false;
		}

		return;
	}

	canvas_write_blocker = false;
	if (current_tool == FILL)
	{
		fill_from(x - x1, y - y1);
		return;
	}

	mouse_held = true;
	if (current_tool == SELECT)
	{
		if (selection_active && selection_rect.point_in_rect_strict(x - x1, y - y1))
		{
			dragging_selection = true;
			draw_selection_rect = false;
		}

		else
			finalize_selection();
	}

	int new_x = x - x1;
	int new_y = y - y1;
	prev_mouse_x = new_x;
	prev_mouse_y = new_y;
	if (current_tool == BRUSH || current_tool == LINE)
		brush_stroke(new_x, new_y, temp_image);

	if (current_tool == ERASER)
	{
		GLubyte br = color_red;
		GLubyte bg = color_green;
		GLubyte bb = color_blue;
		bool bc = circle_brush;
		color_red = 255;
		color_green = 255;
		color_blue = 255;
		circle_brush = false;
		brush_stroke(new_x, new_y, temp_image);
		color_red = br;
		color_green = bg;
		color_blue = bb;
		circle_brush = bc;
	}

	prev_moved_x = x;
	prev_moved_y = y;
}

void Canvas::finalize_selection()
{
	dragging_selection = false;
	selection_active = false;
	write_temp_to_canvas(false);
	clear_temp();
	temp_offset_x = 0;
	temp_offset_y = 0;
	draw_selection_rect = false;
	selection_rect = Rect(0, 0, 0, 0);
}

void Canvas::draw(TextureManager* texture_manager)
{
	draw_line_counter = 0;
	ScreenElement::draw(texture_manager);
	glDisable(GL_TEXTURE_2D);
	//new
	glEnable(GL_STENCIL_TEST);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glDepthMask(GL_FALSE);
	glStencilFunc(GL_NEVER, 1, 0xFF);
	glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
	glStencilMask(0xFF);
	glColor4f(1.0, 0.0, 0.0, 1.0);
	glRectf(x1, y1, x2, y2);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);
	glStencilMask(0x00);
	glStencilFunc(GL_EQUAL, 1, 0xFF);
	//new
	glRasterPos2f(x1, y1);
	glDrawPixels(x2 - x1, y2 - y1, GL_RGBA, GL_UNSIGNED_BYTE, image);
	if (x1 + temp_offset_x < 0 || y1 + temp_offset_y < 0)
	{
		int x_loss = x1 + temp_offset_x < 0 ? x1 + temp_offset_x : 0;
		int y_loss = y1 + temp_offset_y < 0 ? y1 + temp_offset_y : 0;
		int new_temp_width = (int)(x2 - x1) + x_loss;
		int new_temp_height = (int)(y2 - y1) + y_loss;
		glPixelStorei(GL_UNPACK_ROW_LENGTH, x2 - x1);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, -x_loss);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, -y_loss);
		glRasterPos2f(x_loss != 0 ? 0.0 : x1 + temp_offset_x, y_loss != 0 ? 0.0 : y1 + temp_offset_y);
		glDrawPixels(new_temp_width, new_temp_height, GL_RGBA, GL_UNSIGNED_BYTE, temp_image);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
		glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
	}

	else
	{
		glRasterPos2f(x1 + temp_offset_x, y1 + temp_offset_y);
		glDrawPixels(x2 - x1, y2 - y1, GL_RGBA, GL_UNSIGNED_BYTE, temp_image);
	}

	glPushMatrix();
	glTranslatef(x1, y1, 0.0);
	glColor4f(0.0, 0.0, 0.0, 1.0);
	if (draw_selection_rect)
	{
		int ex1 = min(selection_rect.x1, selection_rect.x2);
		int ex2 = max(selection_rect.x1, selection_rect.x2) - 1;
		int ey1 = min(selection_rect.y1, selection_rect.y2);
		int ey2 = max(selection_rect.y1, selection_rect.y2) - 1;
		gl_draw_dotted_line(ex1, ey1, ex2, ey1);
		gl_draw_dotted_line(ex1, ey1, ex1, ey2);
		gl_draw_dotted_line(ex1, ey2, ex2, ey2);
		gl_draw_dotted_line(ex2, ey1, ex2, ey2);
	}

	glColor4f(1.0, 1.0, 1.0, 1.0);
	glPopMatrix();

	glEnable(GL_TEXTURE_2D);
	//new
	glStencilMask(~0);
	glClear(GL_STENCIL_BUFFER_BIT);
	glDisable(GL_STENCIL_TEST);
	//new
}

void Canvas::gl_draw_dotted_line(int from_x, int from_y, int to_x, int to_y)
{
	glDisable(GL_MULTISAMPLE);
	if (to_x < from_x)
		std::swap(from_x, to_x);

	if (to_y < from_y)
		std::swap(from_y, to_y);

	int xvar = from_x;
	int yvar = from_y;
	int* inc_var = from_x == to_x ? &yvar : &xvar;
	int target = from_x == to_x ? to_y : to_x;
	int counter = 0;
	for (; *inc_var <= target; ++(*inc_var))
	{
		if (counter == 3)
		{
			if (selection_active)
			{
				glColor4f(1.0, 1.0, 1.0, 1.0);
				glBegin(GL_POINTS);
				glVertex2f(xvar, yvar);
				glEnd();
			}

			counter++;
			continue;
		}

		if (counter == 4)
		{
			if (selection_active)
			{
				glColor4f(1.0, 1.0, 1.0, 1.0);
				glBegin(GL_POINTS);
				glVertex2f(xvar, yvar);
				glEnd();
			}

			counter = 0;
			continue;
		}

		if (!selection_active)
		{
			auto color = get_color(xvar, yvar);
			glColor4f(color.x < 127 ? 255 : 0, color.y < 127 ? 255 : 0, color.z < 127 ? 255 : 0, 1.0);
		}

		else
			glColor4f(0.0, 0.0, 0.0, 1.0);

		glBegin(GL_POINTS);
		glVertex2f(xvar, yvar);
		glEnd();
		counter++;
	}

	glEnable(GL_MULTISAMPLE);
}

void Canvas::draw_dotted_line(int from_x, int from_y, int to_x, int to_y, GLubyte* pointer)
{
	if (from_x == to_x)
	{
		if (to_y < from_y)
			std::swap(to_y, from_y);

		for (int cur_y = from_y; ; cur_y += 4)
		{
			if (cur_y + 4 >= to_y)
			{
				if (cur_y < to_y)
					draw_line(from_x, cur_y, to_x, to_y, pointer);
				
				break;
			}

			draw_line(from_x, cur_y, to_x, cur_y + 4, pointer);
			cur_y += 8;
		}
	}

	if (from_y == to_y)
	{
		if (to_x < from_x)
			std::swap(to_x, from_x);

		for (int cur_x = from_x;; cur_x += 4)
		{
			if (cur_x + 4 >= to_x)
			{
				if (cur_x < to_x)
					draw_line(cur_x, from_y, to_x, to_y, pointer);

				break;
			}

			draw_line(cur_x, from_y, cur_x + 4, to_y, pointer);
			cur_x += 8;
		}
	}
}

void Canvas::draw_dotted_rectangle(int from_x, int from_y, int to_x, int to_y, GLubyte* pointer, GLubyte local_alpha)
{
	GLubyte br = color_red, bg = color_green, bb = color_blue, ba = color_alpha;
	color_red = color_green = color_blue = 0;
	color_alpha = local_alpha;
	draw_dotted_line(from_x, from_y, to_x, from_y, pointer);
	draw_dotted_line(from_x, from_y, from_x, to_y, pointer);
	draw_dotted_line(from_x, to_y, to_x, to_y, pointer);
	draw_dotted_line(to_x, from_y, to_x, to_y, pointer);
	color_red = br;
	color_green = bg;
	color_blue = bb;
	color_alpha = ba;
}

void Canvas::draw_rectangle(int from_x, int from_y, int to_x, int to_y, GLubyte* pointer, bool fill, bool immediate)
{
	if ((from_x < 0 && to_x < 0) || (from_y < 0 && to_y < 0) || (from_x >= x2 - x1 && to_x >= x2 - x1) || (from_y >= y2 - y1 && to_y >= y2 - y1))
		return;
	
	if (from_x < 0)
		from_x = 0;

	if (from_y < 0)
		from_y = 0;

	if (to_x >= x2 - x1)
		to_x = x2 - x1 - 1;

	if (to_y >= y2 - y1)
		to_y = y2 - y1 - 1;

	if (!fill)
	{
		draw_line(from_x, from_y, to_x, from_y, pointer);
		draw_line(from_x, from_y, from_x, to_y, pointer);
		draw_line(from_x, to_y, to_x, to_y, pointer);
		draw_line(to_x, from_y, to_x, to_y, pointer);
		set_pixel(to_x, to_y, color_red, color_green, color_blue, color_alpha, pointer);
	}

	if (fill)
	{
		int start_x = min(from_x, to_x);
		int end_x = max(from_x, to_x);
		int start_y = min(from_y, to_y);
		int end_y = max(from_y, to_y);
		end_x = min(end_x, (int)(x2 - x1));
		end_y = min(end_y, (int)(y2 - y1));
		start_x = max(0, start_x);
		start_y = max(0, start_y);
		for (int j = start_y; j <= end_y; ++j)
		{
			for (int i = start_x; i <= end_x; ++i)
			{
				int start = j * (x2 - x1) * 4 + i * 4;
				pointer[start] = color_red;
				pointer[start + 1] = color_green;
				pointer[start + 2] = color_blue;
				pointer[start + 3] = color_alpha;
			}
		}
	}
}

void Canvas::draw_ellipse(int center_x, int center_y, int x_radius, int y_radius, GLubyte* pointer, bool fill, bool immediate)
{
	draw_ellipse_counter++;
	if (!fill && brush_size != 1)
	{
		x_radius = abs(x_radius);
		y_radius = abs(y_radius);
		draw_ellipse(center_x, center_y, x_radius, y_radius, pointer, true);
		if ((x_radius - brush_size <= 0) || (y_radius - brush_size <= 0))
			return;

		GLubyte old_alpha = color_alpha;
		color_alpha = 0;
		draw_ellipse(center_x, center_y, x_radius - brush_size, y_radius - brush_size, pointer, true);
		color_alpha = old_alpha;
		return;
	}
	
	if (fill)
	{
		/*x_radius = abs(x_radius);
		y_radius = abs(y_radius);
		int* yvals = (int*)malloc(sizeof(int) * x_radius * 3);
		for (int i = center_x - x_radius + 1; i <= center_x + x_radius - 1; ++i)
		{
			float arg = y_radius * y_radius * (1.0 - ((float)(i - center_x) * (i - center_x) / ((float)x_radius * x_radius)));
			int cur_y = (int)(sqrt(arg)) + center_y;
			yvals[i - (center_x - x_radius + 1)] = cur_y;
		}

		for (int j = center_y - y_radius + 1; j <= center_y + y_radius - 1; ++j)
		{
			int startx = -1;
			int endx = -1;
			int shade_distance = -1;
			for (int i = center_x - x_radius + 1; i <= center_x + x_radius - 1; ++i)
			{
				int cur_y = yvals[i - (center_x - x_radius + 1)];
				if (j <= cur_y && cur_y - 2 * (cur_y - center_y) <= j)
				{
					if (startx == -1)
						startx = i;
				}

				else if (i > center_x - x_radius + 1)
				{
					int prev_y = yvals[i - (center_x - x_radius + 1) - 1];
					if (j <= prev_y && prev_y - 2 * (prev_y - center_y) <= j)
						endx = i - 1;
				}
			}

			for (int i = center_x - x_radius + 1; i <= center_x + x_radius - 1; ++i)
			{
				int cur_y = yvals[i - (center_x - x_radius + 1)];
				if (j <= cur_y && cur_y - 2 * (cur_y - center_y) <= j)
				{
					if (i < 0 || i >= x2 - x1 || j < 0 || j >= y2 - y1)
						continue;

					int start = j * (x2 - x1) * 4 + i * 4;
					pointer[start] = color_red;
					pointer[start + 1] = color_green;
					pointer[start + 2] = color_blue;
					pointer[start + 3] = color_alpha;
				}

				else
				{
					int inc = (j < center_y) ? 1 : -1;
					if ((j + inc) <= cur_y && cur_y - 2 * (cur_y - center_y) <= (j + inc))
					{
						if (shade_distance == -1)
							shade_distance = startx - i; //this should always be positive

						float step = 1.0 / (shade_distance + 1) * color_alpha;
						int start = j * (x2 - x1) * 4 + i * 4;
						pointer[start] = color_red;
						pointer[start + 1] = color_green;
						pointer[start + 2] = color_blue;
						pointer[start + 3] = color_alpha - (startx - i) * step;
					}
				}
			}
		}

		free(yvals);
		return;*/

		x_radius = abs(x_radius);
		y_radius = abs(y_radius);
		int* yvals = (int*)malloc(sizeof(int) * x_radius * 3);
		for (int j = center_y - y_radius + 1; j <= center_y + y_radius - 1; ++j)
		{
			for (int i = center_x - x_radius + 1; i <= center_x + x_radius - 1; ++i)
			{
				int cur_y;
				if (j == center_y - y_radius + 1)
				{
					float arg = y_radius * y_radius * (1.0 - ((float)(i - center_x) * (i - center_x) / ((float)x_radius * x_radius)));
					cur_y = (int)(sqrt(arg)) + center_y;
					yvals[i - (center_x - x_radius + 1)] = cur_y;
				}

				else cur_y = yvals[i - (center_x - x_radius + 1)];
				if (j <= cur_y && cur_y - 2 * (cur_y - center_y) <= j)
				{
					if (i < 0 || i >= x2 - x1 || j < 0 || j >= y2 - y1)
						continue;

					int start = j * (x2 - x1) * 4 + i * 4;
					pointer[start] = color_red;
					pointer[start + 1] = color_green;
					pointer[start + 2] = color_blue;
					pointer[start + 3] = color_alpha;
				}
			}
		}

		free(yvals);
		return;
	}
	
	if (x_radius == 0)
	{
		draw_line(center_x, center_y - y_radius, center_x, center_y + y_radius, pointer);
		return;
	}

	if (y_radius == 0)
	{
		draw_line(center_x - x_radius, center_y, center_x + x_radius, center_y, pointer);
		return;
	}

	x_radius = abs(x_radius);
	int prev_x = center_x - x_radius, prev_y = center_y;
	for (int i = center_x - x_radius; i <= center_x + x_radius; ++i)
	{
		float arg = y_radius * y_radius * (1.0 - ((float)(i - center_x) * (i - center_x) / ((float)x_radius * x_radius)));
		int cur_y = (int)(sqrt(arg)) + center_y;
		if (i == center_x)
			cur_y = prev_y;

		draw_line(prev_x, prev_y, i, cur_y, pointer);
		draw_line(prev_x, prev_y - 2*(prev_y - center_y), i, cur_y - 2*(cur_y - center_y), pointer);
		if (fill)
			draw_line(i, cur_y, i, cur_y - 2 * (cur_y - center_y), pointer);

		prev_x = i;
		prev_y = cur_y;
	}

	set_pixel(center_x + x_radius - 1, center_y, color_red, color_green, color_blue, color_alpha, temp_image);
}

vector<pair<int, int>> Canvas::draw_line(int from_x, int from_y, int to_x, int to_y, GLubyte* pointer)
{
	//cout << from_x << " " << from_y << " " << to_x << " " << to_y << "\n";
	draw_line_counter++;
	vector<pair<int, int>> to_return;
	if (circle_brush && brush_size > 3)
	{
		int old_size = brush_size;
		brush_stroke(from_x, from_y, pointer);
		//color_red = 255;
		brush_stroke(to_x, to_y, pointer);
		int sol1, sol2;
		if (from_x == to_x)
		{
			sol1 = from_x - brush_size;
			sol2 = from_x + brush_size;
		}

		if (from_x == to_x || from_y == to_y)
			brush_size -= 1;

		double center_line_slope = (float)(to_y - from_y) / (float)(to_x - from_x);
		double slope = -1.0 / center_line_slope;
		double y_int = from_y - slope * from_x;
		double a = pow(slope, 2) + 1;
		double b = 2 * slope * (y_int - from_y) - 2 * from_x;
		double part1 = pow(y_int - from_y, 2);
		double part2 = pow(from_x, 2);
		double part3 = pow(brush_size, 2);
		double c = part1 + part2 - part3;
		double discrim = sqrt(b * b - 4 * a * c);
		sol1 = (-b + discrim) / (2 * a);
		sol2 = (-b - discrim) / (2 * a);
		if (from_y == to_y)
			sol1 = sol2 = from_x;

		float arg = brush_size * brush_size * (1.0 - ((float)(sol1 - from_x) * (sol1 - from_x) / ((float)brush_size * brush_size)));
		int cur_y1 = (int)(sqrt(arg)) + from_y;
		arg = brush_size * brush_size * (1.0 - ((float)(sol2 - from_x) * (sol2 - from_x) / ((float)brush_size * brush_size)));
		int cur_y2 = (int)(sqrt(arg)) + from_y;
		if (slope > 0)
			cur_y2 -= 2 * (cur_y2 - from_y);

		else cur_y1 -= 2 * (cur_y1 - from_y);
		circle_brush = false;
		brush_size = 1;
		//color_green = 255;
		auto points = draw_line(sol2, cur_y1, sol2 + (to_x - from_x), cur_y1 + (to_y - from_y), pointer);
		//color_red = 0;
		//color_green = 0;
		//color_blue = 255;
		create_parallel_fill(points, sol1 - sol2, 0, pointer);
		//color_green = 255;
		create_parallel_fill(points, 0, cur_y2 - cur_y1, pointer);
		brush_size = old_size;
		circle_brush = true;
		//color_red = color_green = color_blue = 0;
		return to_return;
	}

	if (brush_size != 1)
	{
		int old_size = brush_size;
		brush_stroke(from_x, from_y, pointer);
		brush_stroke(to_x, to_y, pointer);
		int start_x = from_x;
		int start_y = from_y;
		if (to_x <= from_x)
			start_x -= brush_size;//(brush_size % 2 == 0) ? brush_size / 2 - 1 : brush_size / 2;

		else start_x += brush_size;//brush_size / 2;
		//start_y += copysign(brush_size / 2, to_y - from_y);
		if (to_y < from_y)
			start_y -= brush_size;//(brush_size % 2 == 0) ? brush_size / 2 - 1 : brush_size / 2;

		else start_y += brush_size;//brush_size / 2;
		brush_size = 1;
		auto points = draw_line(start_x, start_y, start_x + (to_x - from_x), start_y + (to_y - from_y), pointer);
		brush_size = old_size;
		create_parallel_fill(points, copysign(brush_size * 2, from_x - to_x), 0, pointer);
		create_parallel_fill(points, 0, copysign(brush_size * 2, from_y - to_y == 0 ? -1 : from_y - to_y), pointer);
		return to_return;
	}
	
	int x_dist = abs(to_x - from_x);
	int y_dist = abs(to_y - from_y);
	int max_dist = max(x_dist, y_dist);
	int min_dist = min(x_dist, y_dist);
	int seg_length;
	if (min_dist == 0)
		seg_length = max_dist;

	else
		seg_length = ceil(((float)max_dist) / ((float)min_dist));

	int target_remove = 0;
	float index_inc;
	float next_index;
	int current_index = 0;
	if (min_dist != 0 && max_dist % min_dist != 0)
	{
		target_remove = min_dist * seg_length - max_dist;
		index_inc = ((float)max_dist) / ((float)target_remove);
		next_index = index_inc;
	}

	int x_inc = copysign(1.0, to_x - from_x);
	int y_inc = copysign(1.0, to_y - from_y);
	int cur_x = from_x, cur_y = from_y;
	int *seg_var, *alt_var;
	int seg_inc, alt_inc, seg_target, alt_target;
	if (max_dist == x_dist)
	{
		seg_var = &cur_x, alt_var = &cur_y;
		seg_inc = x_inc, alt_inc = y_inc, seg_target = to_x, alt_target = to_y;
	}

	else
	{
		seg_var = &cur_y, alt_var = &cur_x;
		seg_inc = y_inc, alt_inc = x_inc, seg_target = to_y, alt_target = to_x;
	}

	if (*alt_var == alt_target)
		alt_inc = 0;

	do
	{
		for (int i = 0; i < seg_length; ++i)
		{
			if (target_remove <= 0)
			{
				brush_stroke(cur_x, cur_y, pointer);
				to_return.push_back(pair<int, int>(cur_x, cur_y));
				*seg_var += seg_inc;
			}

			else
			{
				current_index++;
				if (current_index == floor(next_index))
				{
					current_index--;
					target_remove--;
					next_index += index_inc;
				}

				else
				{
					brush_stroke(cur_x, cur_y, pointer);
					to_return.push_back(pair<int, int>(cur_x, cur_y));
					*seg_var += seg_inc;
				}
			}
		}

		*alt_var += alt_inc;
	} while (*alt_var != alt_target);

	for (;;)
	{
		brush_stroke(cur_x, cur_y, pointer);
		to_return.push_back(pair<int, int>(cur_x, cur_y));
		if (*seg_var == seg_target)
			break;

		*seg_var += seg_inc;
	}

	return to_return;
}

void Canvas::create_parallel_fill(vector<pair<int, int>> points, int xtrans, int ytrans, GLubyte* pointer)
{
	if (xtrans != 0)
	{
		int sign = 0;
		if (xtrans > 0)
			sign = 1;

		else sign = -1;
		for (int i = 0; i < points.size(); ++i)
		{
			auto point = points[i];
			for (int j = 1; j <= abs(xtrans); ++j)
			{
				int y = point.second;
				int x = point.first + sign * j;
				if (x < 0 || x >= x2 - x1 || y < 0 || y >= y2 - y1)
					continue;

				int start = y * (x2 - x1) * 4 + x * 4;
				pointer[start] = color_red;
				pointer[start + 1] = color_green;
				pointer[start + 2] = color_blue;
				pointer[start + 3] = color_alpha;
			}
		}
	}

	if (ytrans != 0)
	{
		/*vector<pair<int, int>> line2, top_line, bottom_line, left_line, right_line;
		for (int i = 0; i < points.size(); ++i)
			line2.push_back(pair<int, int>(points[i].first, points[i].second + ytrans));

		if (ytrans >= 0)
		{
			top_line = line2;
			bottom_line = points;
		}

		else
		{
			top_line = points;
			bottom_line = line2;
		}

		if (points.back().second >= points[0].second)
		{
			left_line = top_line;
			right_line = bottom_line;
		}

		else
		{
			left_line = bottom_line;
			right_line = top_line;
		}*/

		int sign = 0;
		if (ytrans > 0)
			sign = 1;

		else sign = -1;
		for (int i = 0; i < points.size(); ++i)
		{
			auto point = points[i];
			for (int j = 1; j <= abs(ytrans); ++j)
			{
				int y = point.second + sign * j;
				int x = point.first;
				if (x < 0 || x >= x2 - x1 || y < 0 || y >= y2 - y1)
					continue;

				int start = (point.second + sign * j) * (x2 - x1) * 4 + point.first * 4;
				pointer[start] = color_red;
				pointer[start + 1] = color_green;
				pointer[start + 2] = color_blue;
				pointer[start + 3] = color_alpha;
			}
		}
	}
}

void Canvas::mouse_moved(int x, int y)
{
	if ((current_tool == BRUSH || current_tool == ERASER || current_tool == LINE || current_tool == STICKY_LINES) && !mouse_held && mouse_on && !being_resized && !parent->cursor_lock)
	{
		clear_temp();
		GLubyte br, bg, bb;
		bool bc;
		if (current_tool == ERASER)
		{
			br = color_red;
			bg = color_green;
			bb = color_blue;
			bc = circle_brush;
			color_red = 255;
			color_green = 255;
			color_blue = 255;
			circle_brush = false;
		}

		brush_stroke(x - x1, y - y1, temp_image);
		if (current_tool == ERASER)
		{
			color_red = br;
			color_green = bg;
			color_blue = bb;
			circle_brush = bc;
		}

		prev_moved_x = x;
		prev_moved_y = y;
	}

	if (current_tool == SELECT && !mouse_held && mouse_on && !being_resized)
	{
		if (selection_rect.point_in_rect_strict(x - x1, y - y1))
			parent->set_cursor("cursormove.png");

		else if (!dragging_selection)
			parent->set_cursor("brushcursor.png");
	}
	
	if (!mouse_held /*|| !mouse_on*/)
		return;

	if (current_tool == BRUSH)
	{
		int new_x = x - x1;
		int new_y = y - y1;
		draw_line(prev_mouse_x, prev_mouse_y, new_x, new_y, temp_image);
		prev_mouse_x = new_x;
		prev_mouse_y = new_y;
	}

	else if (current_tool == ERASER)
	{
		GLubyte br = color_red;
		GLubyte bg = color_green;
		GLubyte bb = color_blue;
		bool bc = circle_brush;
		color_red = 255;
		color_green = 255;
		color_blue = 255;
		circle_brush = false;
		int new_x = x - x1;
		int new_y = y - y1;
		draw_line(prev_mouse_x, prev_mouse_y, new_x, new_y, temp_image);
		prev_mouse_x = new_x;
		prev_mouse_y = new_y;
		color_red = br;
		color_green = bg;
		color_blue = bb;
		circle_brush = bc;
	}

	else if (current_tool == LINE)
	{
		clear_temp();
		draw_line(prev_mouse_x, prev_mouse_y, x - x1, y - y1, temp_image);
	}

	else if (current_tool == STICKY_LINES)
	{
		draw_line(prev_mouse_x, prev_mouse_y, x - x1, y - y1, temp_image);
	}

	else if (current_tool == ELLIPSE)
	{
		clear_temp();
		draw_ellipse(prev_mouse_x, prev_mouse_y, x - x1 - prev_mouse_x, y - y1 - prev_mouse_y, temp_image);
	}

	else if (current_tool == RECTANGLE)
	{
		clear_temp();
		draw_rectangle(prev_mouse_x, prev_mouse_y, x - x1, y - y1, temp_image);
	}

	else if (current_tool == FILLED_RECTANGLE)
	{
		clear_temp();
		draw_rectangle(prev_mouse_x, prev_mouse_y, x - x1, y - y1, temp_image, true);
	}

	else if (current_tool == FILLED_ELLIPSE)
	{
		clear_temp();
		draw_ellipse(prev_mouse_x, prev_mouse_y, x - x1 - prev_mouse_x, y - y1 - prev_mouse_y, temp_image, true);
	}

	else if (current_tool == SELECT)
	{
		if (dragging_selection)
		{
			int new_x = x - x1;
			int new_y = y - y1;
			temp_offset_x += new_x - prev_mouse_x;
			temp_offset_y += new_y - prev_mouse_y;
			selection_rect.x1 += new_x - prev_mouse_x;
			selection_rect.x2 += new_x - prev_mouse_x;
			selection_rect.y1 += new_y - prev_mouse_y;
			selection_rect.y2 += new_y - prev_mouse_y;
			prev_mouse_x = new_x;
			prev_mouse_y = new_y;
			return;
		}
		
		clear_temp();
		int actual_x, actual_y;
		actual_x = x < x1 ? x1 : (x > x2 ? x2 : x);
		actual_y = y < y1 ? y1 : (y > y2 ? y2 : y);
		selection_rect = Rect(prev_mouse_x, prev_mouse_y, actual_x - x1, actual_y - y1);
		draw_selection_rect = true;
		/*if (actual_x == x1)
			actual_x++;

		if (actual_x == x2)
			actual_x--;

		if (actual_y == y1)
			actual_y++;

		if (actual_y == y2)
			actual_y--;*/

		//draw_dotted_rectangle(prev_mouse_x, prev_mouse_y, actual_x - x1, actual_y - y1, temp_image);
	}

	prev_moved_x = x;
	prev_moved_y = y;
}

Vector3int Canvas::rgb_to_hsl(float r, float g, float b)
{
	float R = r / 255;
	float G = g / 255;
	float B = b / 255;
	float M = max(max(R, G), B);
	float m = min(min(R, G), B);
	float C = M - m;
	float Hp;
	if (C == 0)
		Hp = 0;

	else if (M == R)
		Hp = fmodf(((G - B) / C), 6);

	else if (M == G)
		Hp = ((B - R) / C) + 2;

	else if (M == B)
		Hp = ((R - G) / C) + 4;

	float H = 60 * Hp;
	float L = .5*(M + m);
	float S;
	if (C == 0)
		S = 0;

	else
		S = C / (1 - abs(2 * L - 1));

	return Vector3int(H < 0 ? 360 + H : H, S * 100.0, L * 100.0);
}

Vector3int Canvas::hsl_to_rgb(float hue, float sat, float lum)
{
	sat /= 100.0;
	lum /= 100.0;
	float C = (1 - abs(2 * lum - 1)) * sat;
	float Hp = hue / 60;
	float X = C * (1 - abs(fmodf(Hp, 2) - 1));
	float R1, G1, B1;
	if (0 <= Hp && Hp < 1)
	{
		R1 = C;
		G1 = X;
		B1 = 0;
	}

	if (1 <= Hp && Hp < 2)
	{
		R1 = X;
		G1 = C;
		B1 = 0;
	}

	if (2 <= Hp && Hp < 3)
	{
		R1 = 0;
		G1 = C;
		B1 = X;
	}

	if (3 <= Hp && Hp < 4)
	{
		R1 = 0;
		G1 = X;
		B1 = C;
	}

	if (4 <= Hp && Hp < 5)
	{
		R1 = X;
		G1 = 0;
		B1 = C;
	}

	if (5 <= Hp && Hp < 6)
	{
		R1 = C;
		G1 = 0;
		B1 = X;
	}

	float m = lum - 0.5*C;
	return Vector3int(255*(R1 + m), 255*(G1 + m), 255*(B1 + m));
}

Canvas::~Canvas()
{
	delete[] image;
	delete[] temp_image;
	delete[] backup_image;
}