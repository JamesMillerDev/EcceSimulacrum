//#include <Windows.h>
#include "fonts.h"
#include "GameParams.h"
#include "Computer.h"

using namespace std;

FT_Library library;
FT_Face face;

void init_fonts()
{
	FT_Error error = FT_Init_FreeType( &library ); //FREE THIS
	if (error)
	{
		exit(1);
	}

	error = FT_New_Face(library, "LiberationSans-Regular.ttf", 0, &face); //AND THIS
	if (error)
	{
		exit(1);
	}
}

//TODO drawing at 32 then at 64 or 48 makes all text look bad forever
vector<float> draw_string(TextureManager* texture_manager, int points, std::string str, float x, float y, bool display, bool highlight, int box_height, bool no_scale)
{
	vector<float> string_widths;
	//HDC hdc = GetDC(NULL);
	FT_Error error = FT_Set_Char_Size(face, 0, points * 30, 0, 0); //TODO: get real DPI
	if (error)
	{
		exit(1);
	}

	if (highlight && str != "")
	{
		vector<float> widths = draw_string(texture_manager, points, str, x, y, false, false);
		float xend = widths[widths.size() - 1];
		glDisable(GL_TEXTURE_2D);
		glColor4f(0.0, 120.0 / 255.0, 215.0 / 255.0, 1.0);
		if (!no_scale)
		{
			glBegin(GL_QUADS);
			glVertex2f(scalex(x), scaley(y - 5));
			glVertex2f(scalex(x), scaley(y - 5 + box_height)/*+ face->max_advance_height*/);
			glVertex2f(scalex(x + xend), scaley(y - 5 + box_height)/*+ face->max_advance_height*/);
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

	//ReleaseDC(NULL, hdc);
	float pen = x;
	int previous = 0;
	for (int i = 0; i < str.length(); ++i)
	{
		FT_UInt glyph_index = FT_Get_Char_Index(face, (int) str[i]);
		if (previous)
		{
			FT_Vector delta;
			FT_Get_Kerning(face, previous, glyph_index, FT_KERNING_DEFAULT, &delta);
			pen += delta.x >> 6;
		}

		error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);//FT_LOAD_FORCE_AUTOHINT | FT_LOAD_TARGET_LCD_V); //Are these the best options?
		if (error)
		{
			exit(1);
		}

		error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		if (error)
		{
			exit(1);
		}

		FT_Bitmap bitmap = face->glyph->bitmap;
		float start_x = pen + face->glyph->bitmap_left;
		float start_y = y - (bitmap.rows - face->glyph->bitmap_top);
		string_widths.push_back(pen - x);
		if (display)
		{
			texture_manager->change_character(bitmap, to_string(points) + str[i], highlight);
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

		pen += face->glyph->advance.x >> 6;
		previous = glyph_index;
	}

	string_widths.push_back(pen - x);
	return string_widths;
}