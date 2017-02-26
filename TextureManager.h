#ifndef TEXTURE_MANAGER
#define TEXTURE_MANAGER

#include <map>
#include <unordered_map>
#include <string>
#include "IL/il.h"
#include "RGBpixmap.h"
#include "GameParams.h"
#include "ft2build.h"
#include FT_FREETYPE_H

using std::string;
using std::map;

class TextureManager
{
private:
	GLuint* textures;
	GLuint* char_texture;
	static map<string, int> height_map;
	static map<string, int> width_map;
	int current_index;
	int max_textures;
	unsigned char* pixel_array; //For drawing characters from fonts
	
public:
	TextureManager(int num_textures);
	~TextureManager();
	void load_texture(string name, bool mips = false);
	void change_texture(string name);
	void change_texture(int id);
	void change_character(FT_Bitmap bitmap, string c, bool highlight = false);
	static int get_height(string name);
	static int get_width(string name);
	unordered_map<string, int> index_map;
};

#endif