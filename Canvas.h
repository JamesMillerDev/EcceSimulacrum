#ifndef CANVAS
#define CANVAS

#include "Computer.h"
#include "ScrollPanel.h"
#include "Button.h"
#include "Vector3.h"

struct ColorPickerButton;

struct ShapeInstruction
{
	int type;
	int p1, p2, p3, p4;
	bool fill;
	GLubyte* pointer;
	ShapeInstruction(int t, int _p1, int _p2, int _p3, int _p4, bool _fill, GLubyte* _pointer) : type(t), p1(_p1), p2(_p2), p3(_p3), p4(_p4), fill(_fill), pointer(_pointer) {}
};

struct Canvas : public ScreenElement
{
	Computer* parent;
	vector<ScreenElement*> child_components;
	vector<Button*> tools;
	vector<ShapeInstruction> instructions;
	TextField *h_box, *s_box, *l_box, *r_box, *g_box, *b_box;
	TextField* size_box;
	TextField* alpha_box;
	enum CurrentTool { BRUSH, LINE, FILL, ELLIPSE, SELECT, RECTANGLE, FILLED_RECTANGLE, FILLED_ELLIPSE, STICKY_LINES, COLOR_PICKER, ERASER };
	CurrentTool current_tool = BRUSH;
	Canvas(float _x1, float _y1, float _x2, float _y2, string _name, Computer* _parent, Application _application);
	bool mouse_on = false;
	bool mouse_held = false;
	bool did_backup = false;
	bool circle_brush = false;
	bool being_resized = false;
	bool canvas_write_blocker = false;
	void mouse_over(int x, int y);
	void mouse_off();
	void mouse_clicked(int button, int state, int x, int y);
	void mouse_moved(int x, int y);
	void resize(int nx1, int ny1, int nx2, int ny2, bool backup = true);
	void animate();
	GLubyte* image;
	GLubyte* temp_image;
	GLubyte* backup_image;
	GLubyte color_red = 0;
	GLubyte color_green = 0;
	GLubyte color_blue = 0;
	GLubyte color_alpha = 255;
	vector<ColorPickerButton*> color_history;
	void push_color(bool use_color = false, GLubyte red = 0, GLubyte green = 0, GLubyte blue = 0);
	void draw(TextureManager* texture_manager);
	vector<pair<int, int>> draw_line(int from_x, int from_y, int to_x, int to_y, GLubyte* pointer);
	void draw_ellipse(int center_x, int center_y, int x_radius, int y_radius, GLubyte* pointer, bool fill = false, bool immediate = false);
	void draw_dotted_rectangle(int from_x, int from_y, int to_x, int to_y, GLubyte* pointer, GLubyte local_alpha = 255);
	void draw_rectangle(int from_x, int from_y, int to_x, int to_y, GLubyte* pointer, bool fill = false, bool immediate = false);
	void draw_dotted_line(int from_x, int from_y, int to_x, int to_y, GLubyte* pointer);
	void change_tool(string name);
	int prev_mouse_x;
	int prev_mouse_y;
	int prev_moved_x = 0;
	int prev_moved_y = 0;
	int backup_width, backup_height;
	void set_pixel(int x, int y, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLubyte* pointer = NULL);
	void press_key(unsigned char key);
	void release_key(unsigned char key);
	vector<pair<int, int>> fill_from(int x, int y, GLubyte* pointer = NULL);
	Vector4 get_color(int x, int y, GLubyte* pointer = NULL);
	int brush_size = 1;
	void brush_stroke(int x, int y, GLubyte* pointer);
	Rect selection_rect = Rect(0, 0, 0, 0);
	bool selection_active;
	bool dragging_selection;
	bool draw_selection_rect = false;
	bool increasing_size = false;
	bool decreasing_size = false;
	bool increasing_red = false;
	bool decreasing_red = false;
	bool increasing_green = false;
	bool decreasing_green = false;
	bool increasing_blue = false;
	bool decreasing_blue = false;
	bool temp_non_zero();
	void write_temp_to_canvas(bool backup = true);
	void clear_temp();
	void save_file();
	void load_file();
	void finalize_selection();
	void gl_draw_dotted_line(int from_x, int from_y, int to_x, int to_y);
	void load_image_file(string file_name, string folder_name);
	void make_backup();
	void undo();
	void create_parallel_fill(vector<pair<int, int>> points, int xtrans, int ytrans, GLubyte* pointer);
	Vector3int rgb_to_hsl(float r, float g, float b);
	Vector3int hsl_to_rgb(float hue, float sat, float lum);
	int temp_offset_x = 0;
	int temp_offset_y = 0;
	~Canvas();
	int draw_ellipse_counter = 0;
	int draw_line_counter = 0;
	void change_size(int delta);
	void change_opacity(int delta);
};

#endif