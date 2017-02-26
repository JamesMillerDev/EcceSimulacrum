#ifndef CANVAS
#define CANVAS

#include "Computer.h"
#include "ScrollPanel.h"
#include "Button.h"
#include "Vector3.h"

struct Canvas : public ScreenElement
{
	Computer* parent;
	vector<ScreenElement*> child_components;
	vector<Button*> tools;
	TextField *h_box, *s_box, *l_box, *r_box, *g_box, *b_box;
	TextField* size_box;
	TextField* alpha_box;
	enum CurrentTool { BRUSH, LINE, FILL, ELLIPSE, SELECT, RECTANGLE, FILLED_RECTANGLE, FILLED_ELLIPSE, STICKY_LINES, COLOR_PICKER };
	CurrentTool current_tool = BRUSH;
	Canvas(float _x1, float _y1, float _x2, float _y2, string _name, Computer* _parent, Application _application);
	bool mouse_on = false;
	bool mouse_held = false;
	void mouse_over(int x, int y);
	void mouse_off();
	void mouse_clicked(int button, int state, int x, int y);
	void mouse_moved(int x, int y);
	void resize(int nx1, int ny1, int nx2, int ny2);
	GLubyte* image;
	GLubyte* temp_image;
	GLubyte color_red = 0;
	GLubyte color_green = 0;
	GLubyte color_blue = 0;
	GLubyte color_alpha = 255;
	void draw(TextureManager* texture_manager);
	void draw_line(int from_x, int from_y, int to_x, int to_y, GLubyte* pointer);
	void draw_ellipse(int center_x, int center_y, int x_radius, int y_radius, GLubyte* pointer);
	void draw_dotted_rectangle(int from_x, int from_y, int to_x, int to_y, GLubyte* pointer, GLubyte local_alpha = 255);
	void draw_rectangle(int from_x, int from_y, int to_x, int to_y, GLubyte* pointer, bool fill = false);
	void draw_dotted_line(int from_x, int from_y, int to_x, int to_y, GLubyte* pointer);
	void change_tool(string name);
	int prev_mouse_x;
	int prev_mouse_y;
	int prev_moved_x = 0;
	int prev_moved_y = 0;
	void set_pixel(int x, int y, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLubyte* pointer = NULL);
	void press_key(unsigned char key);
	void fill_from(int x, int y);
	Vector4 get_color(int x, int y, GLubyte* pointer = NULL);
	int brush_size = 1;
	void brush_stroke(int x, int y, GLubyte* pointer);
	Rect selection_rect = Rect(0, 0, 0, 0);
	bool selection_active;
	bool dragging_selection;
	void write_temp_to_canvas();
	void clear_temp();
	void save_file();
	void load_file();
	void finalize_selection();
	Vector3int rgb_to_hsl(float r, float g, float b);
	Vector3int hsl_to_rgb(float hue, float sat, float lum);
	int temp_offset_x = 0;
	int temp_offset_y = 0;
	~Canvas();
};

#endif