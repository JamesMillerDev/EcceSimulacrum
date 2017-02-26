#ifndef FONTS
#define FONTS

#include <string>
#include <vector>
#include <ft2build.h>
#include FT_FREETYPE_H
#include "TextureManager.h"

void init_fonts();
void load_all_characters();
std::vector<float> draw_string(TextureManager* texture_manager, int size, std::string c, float x, float y, bool display = true, bool highlight = false, int box_height = 0, bool no_scale = false);

#endif