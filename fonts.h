#ifndef FONTS
#define FONTS

#include <string>
#include <vector>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "TextureManager.h"

void init_fonts();
void change_font(string name);
std::vector<float> draw_string(TextureManager* texture_manager, int size, std::string c, int x, int y, bool display = true, bool highlight = false, int box_height = 0, bool no_scale = false, bool load = false, bool give_widths = true, bool give_height = false);
void draw_string_bounded(TextureManager* texture_manager, int size, std::string str, int x, int y, int end_x, int spacing);
void right_justify(TextureManager* texture_manager, int size, std::string c, int x, int y, bool display = true, bool highlight = false, int box_height = 0, bool no_scale = false, bool load = false, bool give_widths = true);
pair<int, int> get_top_and_bottom(std::string str, int points);
void get_string_widths(int points, std::string str, float* arr);

#endif