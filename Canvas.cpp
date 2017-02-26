#include <algorithm>
#include "Canvas.h"
#include "Button.h"
#include "Files.h"
#include "ColorPickerButton.h"
#include "InfoText.h"

//TODO fill canvas with black, see white bar when minimizing (doesn't seem to be happening now...)
//TODO 250, 157, 204 this color brings up the wrong HSL box
//TODO moving scroll wheel always generates two click events?
//TODO severe framerate problems with fill in genral, especially filled rectangle tool

//TODO get "directory does not exist" error on save then click ok, closes whole dialogue
//TODO make right clicking outside of canvas cancel tool
//TODO bring up color picker dialog, minimize, try to bring app back up, crashes
//TODO clicking middle mouse button cuts things off early while using a tool?
//TODO cursors e.g. for move tool
//TODO power down computer while color picker box is open, framerate is very low
//TODO shouldn't be able to click on any toolbar buttons while color picker box is open
//TODO typing in too big a size in the size box freezes the game
//TODO do selection from very bottom left corner, some dotted rectangle is left behind after selection cleared

//TODO undo
//TODO add undo and color picker to toolbar

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
	for (int i = 0; i < (x2 - x1) * (y2 - y1) * 4; ++i)
	{
		image[i] = 255;
		temp_image[i] = 0;
	}

	auto color_picker_button = make_unique<ColorPickerButton>(20.0, 1004.0, 56.0, 1040.0, "", PAINT, this);
	parent->to_be_added.push_back(std::move(color_picker_button));
	auto size_field = make_unique<TextField>(490 + 180 + 80, 1004 - 7, 530 + 180 + 80, 1040 - 7, "textfield.png", parent, PAINT);
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
			this->size_box->text = "1";
		}

		if (value > 100)
		{
			value = 100;
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
	parent->to_be_added.push_back(make_unique<InfoText>(440 + 180 + 80, 1015-7, 480 + 180 + 80, 1015-7, "invisible.png", "Size:", PAINT));
	auto alpha_field = make_unique<TextField>(600 + 180 + 80, 1004-7, 680 + 180 + 80, 1040-7, "textfield.png", parent, PAINT);
	parent->to_be_added.push_back(make_unique<InfoText>(525 + 180 + 80, 1015-7, 485 + 180 + 80, 1015-7, "invisible.png", "Opacity:", PAINT));
	parent->to_be_added.push_back(make_unique<InfoText>(672 + 180 + 80, 1015-7, 672 + 180 + 80, 1015-7, "invisible.png", "%", PAINT));
	alpha_box = alpha_field.get();
	alpha_box->text = to_string((int)((float)color_alpha / 255.0 * 100));
	alpha_box->allowed_characters = allowed;
	alpha_box->take_focus_function = [this]()
	{
		int value = atoi(this->alpha_box->text.c_str());
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

void Canvas::change_tool(string name)
{
	if (current_tool == SELECT)
		finalize_selection();

	if (name == "ellipse")
		current_tool = ELLIPSE;

	else if (name == "fill")
		current_tool = FILL;

	else if (name == "filledrectangle")
		current_tool = FILLED_RECTANGLE;

	else if (name == "line")
		current_tool = LINE;

	else if (name == "paintbrush")
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
		if (tools[i]->name == name + ".png")
			tools[i]->dark = true;

		else tools[i]->dark = false;
	}
}

void Canvas::resize(int nx1, int ny1, int nx2, int ny2)
{
	GLubyte* new_image = new GLubyte[(int)(nx2 - nx1) * (int)(ny2 - ny1) * 4];
	//for (int j = ny2 - ny1 - 1; j >= 0; --j)
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
	switch (brush_size)
	{
	case 1:
		set_pixel(x, y, color_red, color_green, color_blue, color_alpha, pointer);
		break;

	/*case 2:
		set_pixel(x, y, color_red, color_green, color_blue, color_alpha, pointer);
		set_pixel(x - 1, y, color_red, color_green, color_blue, color_alpha, pointer);
		set_pixel(x + 1, y, color_red, color_green, color_blue, color_alpha, pointer);
		set_pixel(x, y + 1, color_red, color_green, color_blue, color_alpha, pointer);
		set_pixel(x, y - 1, color_red, color_green, color_blue, color_alpha, pointer);
		break;*/

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

		for (int i = start_x; i <= x + brush_size / 2; ++i)
		{
			for (int j = start_y; j <= y + brush_size / 2; ++j)
				set_pixel(i, j, color_red, color_green, color_blue, color_alpha, pointer);
		}
	}
}

void Canvas::clear_temp()
{
	memset(temp_image, 0, (x2 - x1) * (y2 - y1) * 4 * sizeof(GLubyte));
}

void Canvas::press_key(unsigned char key)
{
	//if (current_tool == SELECT)
	//	write_temp_to_canvas(); maybe important when we have real tool switching
	
	clear_temp();
	temp_offset_x = 0;
	temp_offset_y = 0;
	selection_active = false;
	dragging_selection = false; //TODO do this on every tool change
	if (key == 'w')
		brush_size = 1;

	if (key == 'e')
		brush_size = 2;

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
		current_tool = RECTANGLE;

	if (key == 'd')
		current_tool = FILLED_RECTANGLE;

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
}

void Canvas::save_file()
{
	parent->start_application(FILES, "", true);
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

		files_ptr->write_image_file(image, (int)(x2 - x1), (int)(y2 - y1));
		this->parent->close_application(INFO_BOX);
	}, []() {});
	auto name_box = make_unique<TextField>(945 - 400, 394 - 120 + 8 + 10, 945, 394 - 120 + 8 + 60 - 10, "textfield.png", parent, INFO_BOX);
	files_ptr->name_field_ptr = name_box.get();
	parent->to_be_added.insert(parent->to_be_added.end() - 1, std::move(files));
	parent->to_be_added.push_back(std::move(name_box));
	parent->to_be_added.push_back(std::move(button));
	parent->to_be_added.push_back(make_unique<InfoText>(770 - 300, 394 - 120 + 8 + 10 + 15, 770 - 300, 394 - 120 + 8 + 10 + 15, "invisible.png", "Name:"));
}

void Canvas::load_file()
{
	parent->start_application(FILES, "", true);
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

		files_ptr->load_image_file(this);
		this->parent->close_application(INFO_BOX);
	}, []() {});
	auto name_box = make_unique<TextField>(945 - 400, 394 - 120 + 8 + 10, 945, 394 - 120 + 8 + 60 - 10, "textfield.png", parent, INFO_BOX);
	files_ptr->name_field_ptr = name_box.get();
	parent->to_be_added.insert(parent->to_be_added.end() - 1, std::move(files));
	parent->to_be_added.push_back(std::move(name_box));
	parent->to_be_added.push_back(std::move(button));
	parent->to_be_added.push_back(make_unique<InfoText>(770 - 300, 394 - 120 + 8 + 10 + 15, 770 - 300, 394 - 120 + 8 + 10 + 15, "invisible.png", "Name:"));
}

void Canvas::mouse_over(int x, int y)
{
	mouse_on = true;
}

void Canvas::mouse_off()
{
	mouse_on = false;
}

void Canvas::fill_from(int x, int y)
{
	/*vector<pair<int, int>> active_points;
	int start = y * (y2 - y1) * 4 + x * 4;
	Vector4 target_color = get_color(x, y);
	set_pixel(x, y, color_red, color_green, color_blue, color_alpha);
	active_points.push_back(pair<int, int>(x, y));
	vector<pair<int, int>> new_points;
	new_points.reserve((int)(x2 - x1) * (int)(y2 - y1) * 4);
	while (!active_points.empty())
	{
		int j = active_points.size();
		for (int i = 0; i < j; ++i)
		{
			pair<int, int> active_point = active_points[i];
			int cur_x = active_point.first, cur_y = active_point.second;
			Vector4 new_color;

				new_color = get_color(cur_x, cur_y - 1);
				if (new_color.x == target_color.x && new_color.y == target_color.y && new_color.z == target_color.z)
				{
					set_pixel(cur_x, cur_y - 1, color_red, color_green, color_blue, color_alpha);
					new_points.push_back(pair<int, int>(cur_x, cur_y - 1));
				}

				new_color = get_color(cur_x - 1, cur_y);
				if (new_color.x == target_color.x && new_color.y == target_color.y && new_color.z == target_color.z)
				{
					set_pixel(cur_x - 1, cur_y, color_red, color_green, color_blue, color_alpha);
					new_points.push_back(pair<int, int>(cur_x - 1, cur_y));
				}

				new_color = get_color(cur_x + 1, cur_y);
				if (new_color.x == target_color.x && new_color.y == target_color.y && new_color.z == target_color.z)
				{
					set_pixel(cur_x + 1, cur_y, color_red, color_green, color_blue, color_alpha);
					new_points.push_back(pair<int, int>(cur_x + 1, cur_y));
				}

				new_color = get_color(cur_x, cur_y + 1);
				if (new_color.x == target_color.x && new_color.y == target_color.y && new_color.z == target_color.z)
				{
					set_pixel(cur_x, cur_y + 1, color_red, color_green, color_blue, color_alpha);
					new_points.push_back(pair<int, int>(cur_x, cur_y + 1));
				}

		}

		active_points = new_points;
		new_points.clear();
	}*/

	vector<pair<int, int>> active_points;
	Vector4 target_color = get_color(x, y);
	if (target_color.x == color_red && target_color.y == color_green && target_color.z == color_blue)
		return;

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
				auto cur_color = get_color(cur_x, cur_y);
				if (cur_color.x == target_color.x && cur_color.y == target_color.y && cur_color.z == target_color.z)
					set_pixel(cur_x, cur_y, color_red, color_green, color_blue, color_alpha);

				auto color_above = get_color(cur_x, cur_y + 1);
				auto color_below = get_color(cur_x, cur_y - 1);
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
				auto next_color = get_color(cur_x, cur_y);
				if (!(next_color.x == target_color.x && next_color.y == target_color.y && next_color.z == target_color.z))
					done = true;
			}

			cur_x = active_point.first; //active_point.first - 1;
			/*auto color_above = get_color(active_point.first, active_point.second + 1);
			auto color_below = get_color(active_point.first, active_point.second - 1);
			if (color_above.x == target_color.x && color_above.y == target_color.y && color_above.z == target_color.z)
				waiting_above = true;

			else
				waiting_above = false;

			if (color_below.x == target_color.x && color_below.y == target_color.y && color_below.z == target_color.z)
				waiting_below = true;

			else
				waiting_below = false;*/
			direction = -1;
		}
	}
}

void Canvas::write_temp_to_canvas()
{
	/*for (int i = 0; i < (x2 - x1) * (y2 - y1) * 4; i += 4)
	{
		if (temp_image[i + 3] != 0)
		{
			image[i] = temp_image[i];
			image[i + 1] = temp_image[i + 1];
			image[i + 2] = temp_image[i + 2];
			image[i + 3] = temp_image[i + 3];
		}
	}*/

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

void Canvas::mouse_clicked(int button, int state, int x, int y)
{
	//TODO currently assuming every tool must click down first before releasing
	/*if (button != GLUT_LEFT && button != 3 && button != 4)
		return;*/

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
		return;
	}

	if (button == 3)
	{
		//TODO went with 5 here because scroll wheel events seem to register twice, does that happen on every machine?
		int num = atoi(alpha_box->text.c_str());
		if (num < 100)
		{
			if (100 - num < 5)
				num = 100;

			else num += 5;
			alpha_box->text = to_string(num);
			color_alpha = ((float)num / 100.0) * 255.0;
		}

		if (current_tool != BRUSH)
			mouse_moved(prev_moved_x, prev_moved_y);

		return;
	}

	if (button == 4)
	{
		int num = atoi(alpha_box->text.c_str());
		if (num >= 5)
			num -= 5;

		else num = 0;
		alpha_box->text = to_string(num);
		color_alpha = ((float)num / 100.0) * 255.0;
		if (current_tool != BRUSH)
			mouse_moved(prev_moved_x, prev_moved_y);

		return;
	}
	
	if (state == GLUT_UP)
	{
		if (current_tool == SELECT && mouse_held)
		{
			mouse_held = false;
			selection_active = true;
			draw_dotted_rectangle(prev_mouse_x, prev_mouse_y, x - x1, y - y1, temp_image); //TODO problem if the mouse is moving too fast?
			if (dragging_selection)
			{
				dragging_selection = false;
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

			for (int i = first_x + 1; i < second_x; ++i)
			{
				for (int j = first_y + 1; j < second_y; ++j)
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
			color_red = color.x;
			color_green = color.y;
			color_blue = color.z;
			return;
		}

		mouse_held = false;
		write_temp_to_canvas();
		clear_temp();
		return;
	}

	if (current_tool == FILL)
	{
		fill_from(x - x1, y - y1);
		return;
	}

	mouse_held = true;
	if (current_tool == SELECT)
	{
		if (selection_active && selection_rect.point_in_rect_strict(x - x1, y - y1))
			dragging_selection = true;

		else
			finalize_selection();
	}

	int new_x = x - x1;
	int new_y = y - y1;
	prev_mouse_x = new_x;
	prev_mouse_y = new_y;
	if (current_tool == BRUSH || current_tool == LINE)
		brush_stroke(new_x, new_y, temp_image);
}

void Canvas::finalize_selection()
{
	dragging_selection = false;
	selection_active = false;
	draw_dotted_rectangle(selection_rect.x1 - temp_offset_x, selection_rect.y1 - temp_offset_y, selection_rect.x2 - temp_offset_x, selection_rect.y2 - temp_offset_y, temp_image, 0);
	write_temp_to_canvas();
	clear_temp();
	temp_offset_x = 0;
	temp_offset_y = 0;
}

void Canvas::draw(TextureManager* texture_manager)
{
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

	glEnable(GL_TEXTURE_2D);
	//new
	glStencilMask(~0);
	glClear(GL_STENCIL_BUFFER_BIT);
	glDisable(GL_STENCIL_TEST);
	//new
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

void Canvas::draw_rectangle(int from_x, int from_y, int to_x, int to_y, GLubyte* pointer, bool fill)
{
	draw_line(from_x, from_y, to_x, from_y, pointer);
	draw_line(from_x, from_y, from_x, to_y, pointer);
	draw_line(from_x, to_y, to_x, to_y, pointer);
	draw_line(to_x, from_y, to_x, to_y, pointer);
	set_pixel(to_x, to_y, color_red, color_green, color_blue, color_alpha, pointer);
	if (fill)
	{
		int start_x = min(from_x, to_x);
		int end_x = max(from_x, to_x);
		//for (int i = start_x; i <= end_x; ++i)
		//	draw_line(i, from_y, i, to_y, pointer);
		int start_y = min(from_y, to_y);
		int end_y = max(from_y, to_y);
		for (int i = start_x; i <= end_x; ++i)
		{
			for (int j = start_y; j <= end_y; ++j)
				set_pixel(i, j, color_red, color_green, color_blue, color_alpha, pointer);
		}
	}
}

//TODO tops are pointy
void Canvas::draw_ellipse(int center_x, int center_y, int x_radius, int y_radius, GLubyte* pointer)
{
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
		prev_x = i;
		prev_y = cur_y;
	}

	set_pixel(center_x + x_radius - 1, center_y, color_red, color_green, color_blue, color_alpha, temp_image);
}

void Canvas::draw_line(int from_x, int from_y, int to_x, int to_y, GLubyte* pointer)
{
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
			if (target_remove > 0)
			{
				current_index++;
				if (current_index == floor(next_index))
				{
					*seg_var -= seg_inc;
					current_index--;
					target_remove--;
					next_index += index_inc;
				}
			}

			brush_stroke(cur_x, cur_y, pointer);
			*seg_var += seg_inc;
		}

		*alt_var += alt_inc;
	} while (*alt_var != alt_target);

	for (; *seg_var != seg_target; *seg_var += seg_inc)
		brush_stroke(cur_x, cur_y, pointer);
}

void Canvas::mouse_moved(int x, int y)
{
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
		/*if (actual_x == x1)
			actual_x++;

		if (actual_x == x2)
			actual_x--;

		if (actual_y == y1)
			actual_y++;

		if (actual_y == y2)
			actual_y--;*/

		draw_dotted_rectangle(prev_mouse_x, prev_mouse_y, actual_x - x1, actual_y - y1, temp_image);
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
		Hp = abs(fmodf(((G - B) / C), 6));

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

	return Vector3int(H, S * 100.0, L * 100.0);
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

//TODO: FIGURE OUT WHY THIS DOESN'T GET CALLED
Canvas::~Canvas()
{
	delete[] image;
	delete[] temp_image;
}