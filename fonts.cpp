//#include <Windows.h>
#include "fonts.h"
#include "GameParams.h"
#include "Computer.h"

using namespace std;

struct GlyphStruct
{
	FT_Bitmap bitmap;
	FT_Int bitmap_left;
	FT_Int bitmap_top;
	FT_Vector advance;
};

FT_Library library;
FT_Face face, face1, face2, face3;
int font_number = 1;
map<string, GlyphStruct*> glyphs;

void init_fonts()
{
	FT_Error error = FT_Init_FreeType( &library ); //TODO FREE THIS
	if (error)
	{
		exit(1);
	}

	error = FT_New_Face(library, "Lato-Regular.ttf", 0, &face); //AND THIS
	if (error)
	{
		exit(1);
	}

	face1 = face;

	error = FT_New_Face(library, "LiberationMono-Bold.ttf", 0, &face2); //AND THIS
	if (error)
	{
		exit(1);
	}

	error = FT_New_Face(library, "Audiowide-Regular.ttf", 0, &face3); //AND THIS
	if (error)
	{
		exit(1);
	}
}

void change_font(string name)
{
	if (name == "Lato-Regular.ttf")
	{
		face = face1;
		font_number = 1;
	}

	if (name == "LiberationMono-Bold.ttf")
	{
		face = face2;
		font_number = 2;
	}

	if (name == "Audiowide-Regular.ttf")
	{
		face = face3;
		font_number = 3;
	}
}

void right_justify(TextureManager* texture_manager, int size, std::string c, int x, int y, bool display, bool highlight, int box_height, bool no_scale, bool load, bool give_widths)
{
	auto widths = draw_string(texture_manager, size, c, x, y, false, highlight, box_height, no_scale, load, give_widths);
	float width = widths.back();
	draw_string(texture_manager, size, c, (int)(x - width), y, display, highlight, box_height, no_scale, load, give_widths);
}

void get_string_widths(int points, std::string str, float* arr)
{
	FT_Set_Char_Size(face, 0, points * 30, 0, 0);
	float pen = 0;
	int previous = 0;
	int i = 0;
	for (i = 0; i < str.length(); ++i)
	{
		char chr = str[i];
		FT_UInt glyph_index = FT_Get_Char_Index(face, (int)chr);
		if (previous)
		{
			FT_Vector delta;
			FT_Get_Kerning(face, previous, glyph_index, FT_KERNING_DEFAULT, &delta);
			pen += delta.x >> 6;
		}

		string key = to_string(font_number) + to_string(points);
		GlyphStruct glyph = glyphs[key][(int)chr];
		arr[i] = pen;
		pen += glyph.advance.x >> 6;
		previous = glyph_index;
	}

	arr[i] = pen;
}

vector<float> draw_string(TextureManager* texture_manager, int points, std::string str, int x, int y, bool display, bool highlight, int box_height, bool no_scale, bool load, bool give_widths, bool give_height)
{
	FT_Set_Char_Size(face, 0, points * 30, 0, 0); //TODO: get real DPI
	vector<float> string_widths;
	if (highlight && str != "" && box_height != 0)
	{
		vector<float> widths = draw_string(texture_manager, points, str, x, y, false, false);
		float xend = widths[widths.size() - 1];
		glDisable(GL_TEXTURE_2D);
		glColor4f(0.0, 120.0 / 255.0, 215.0 / 255.0, 1.0);
		if (!no_scale)
		{
			glBegin(GL_QUADS);
			glVertex2f(scalex(x), scaley(y - 5));
			glVertex2f(scalex(x), scaley(y - 5 + box_height));
			glVertex2f(scalex(x + xend), scaley(y - 5 + box_height));
			glVertex2f(scalex(x + xend), scaley(y - 5));
			glEnd();
		}

		else
		{
			glBegin(GL_QUADS);
			glVertex2f(x, y - 5);
			glVertex2f(x, y - 5 + box_height);
			glVertex2f(x + xend, y - 5 + box_height);
			glVertex2f(x + xend, y - 5);
			glEnd();
		}

		glEnable(GL_TEXTURE_2D);
		glColor4f(1.0, 1.0, 1.0, 1.0);
	}

	float pen = x;
	int previous = 0;
	for (int i = 0; i < str.length(); ++i)
	{
		char chr = str[i];
		if (chr == '\n')
			continue;

		GlyphStruct glyph;
		FT_UInt glyph_index = FT_Get_Char_Index(face, (int)chr);
		string key = to_string(font_number) + to_string(points);
		if (load)
		{
			if (glyphs[key] == 0)
				glyphs[key] = (GlyphStruct*)malloc(128 * sizeof(GlyphStruct));

			FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
			FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
			int index = (int)chr;
			GlyphStruct glyph_struct;
			glyph_struct.advance = face->glyph->advance;
			glyph_struct.bitmap = face->glyph->bitmap;
			glyph_struct.bitmap_left = face->glyph->bitmap_left;
			glyph_struct.bitmap_top = face->glyph->bitmap_top;
			glyphs[key][index] = glyph_struct;
			glyph = glyph_struct;
		}

		else
		{
			glyph = glyphs[key][(int)chr];
		}

		int yoffset = 0;
		if (chr == -107)
		{
			str[i] = '.';
			yoffset = 5;
		}

		if (previous)
		{
			FT_Vector delta;
			FT_Get_Kerning(face, previous, glyph_index, FT_KERNING_DEFAULT, &delta);
			pen += delta.x >> 6;
		}

		FT_Bitmap bitmap = glyph.bitmap;
		float start_x = pen + glyph.bitmap_left;
		float start_y = y - (bitmap.rows - glyph.bitmap_top) + yoffset;
		if (give_widths)
			string_widths.push_back(pen - x);

		if (give_height)
		{
			if (string_widths.empty() || glyph.bitmap_top > string_widths[0])
			{
				string_widths.clear();
				string_widths.push_back(glyph.bitmap_top);
			}
		}

		int array_num = 0;
		if (font_number == 1 && points == 32 && !highlight)
			array_num = 0;

		else if (font_number == 1 && points == 32 && highlight)
			array_num = 1;

		else if (font_number == 1 && points == 64 && highlight)
			array_num = 2;

		else if (font_number == 1 && points == 64 && !highlight)
			array_num = 3;

		else if (font_number == 2)
			array_num = 4;

		else if (font_number == 3 && points == 90)
			array_num = 5;
		
		else array_num = 6;
		if (display || load)
			texture_manager->change_character(bitmap, chr, highlight, array_num, load);

		if (display)
		{
			if (!no_scale)
			{
				glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0); glVertex2f(scalex(start_x), scaley(start_y));
				glTexCoord2f(0.0, 1.0); glVertex2f(scalex(start_x), scaley(start_y + bitmap.rows));
				glTexCoord2f(1.0, 1.0); glVertex2f(scalex(start_x + bitmap.width), scaley(start_y + bitmap.rows));
				glTexCoord2f(1.0, 0.0); glVertex2f(scalex(start_x + bitmap.width), scaley(start_y));
				glEnd();
			}

			else
			{
				glBegin(GL_QUADS);
				glTexCoord2f(0.0, 0.0); glVertex2f(start_x, start_y);
				glTexCoord2f(0.0, 1.0); glVertex2f(start_x, start_y + bitmap.rows);
				glTexCoord2f(1.0, 1.0); glVertex2f(start_x + bitmap.width, start_y + bitmap.rows);
				glTexCoord2f(1.0, 0.0); glVertex2f(start_x + bitmap.width, start_y);
				glEnd();
			}
		}

		pen += glyph.advance.x >> 6;
		previous = glyph_index;
	}

	if (give_widths)
		string_widths.push_back(pen - x);

	return string_widths;
}

void draw_string_bounded(TextureManager* texture_manager, int size, std::string str, int x, int y, int end_x, int spacing)
{
	int current_y = y;
	while (str.length() != 0)
	{
		vector<float> string_widths = draw_string(texture_manager, size, str, x, current_y, false);
		int last_space = 0;
		int i;
		for (i = 0; i < str.length(); ++i)
		{
			if (str[i] == '\n')
			{
				last_space = i;
				break;
			}
			
			if (str[i] == ' ')
				last_space = i;

			if (string_widths[i] >= end_x - x)
				break;
		}

		if (last_space == 0)
			last_space = i;

		if (i == str.length())
			last_space = i;

		string line = str.substr(0, last_space + 1);
		draw_string(texture_manager, size, line, x, current_y);
		str.erase(0, last_space + 1);
		current_y -= spacing;
	}
}

pair<int, int> get_top_and_bottom(string str, int points)
{
	int top = 0;
	int bottom = 0;
	FT_Error error = FT_Set_Char_Size(face, 0, points * 30, 0, 0);
	for (int i = 0; i < str.length(); ++i)
	{
		FT_UInt glyph_index = FT_Get_Char_Index(face, (int)str[i]);
		FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
		FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		FT_Bitmap bitmap = face->glyph->bitmap;
		if (face->glyph->bitmap_top > top)
			top = face->glyph->bitmap_top;

		if (bitmap.rows - face->glyph->bitmap_top < bottom)
			bottom = bitmap.rows - face->glyph->bitmap_top;
	}

	return pair<int, int>(top, bottom);
}