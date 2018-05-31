#ifndef TEXTURE_MANAGER
#define TEXTURE_MANAGER

#include <map>
#include <unordered_map>
#include <string>
#include "IL/il.h"
#include "IL/ilu.h"
#include "RGBpixmap.h"
#include "GameParams.h"
#include "ft2build.h"
#include FT_FREETYPE_H

using std::string;
using std::map;

class TextureManager
{
private:
	static map<string, int> height_map;
	static map<string, int> width_map;
	map<string, GLubyte*> pixel_data;
	int current_index;
	int max_textures;
	unsigned char* pixel_array; //For drawing characters from fonts
	GLuint liberation_small[128];
	GLuint liberation_small_highlight[128];
	GLuint liberation_big_highlight[128];
	GLuint liberation_big[128];
	GLuint mono_small_highlight[128];
	GLuint audiowide[128];
	GLuint audiowide_big[128];
	
public:
	GLuint* textures;
	TextureManager(int num_textures);
	~TextureManager();
	void load_texture(string name, bool mips = false, bool retain_pixel_data = false, GLubyte* use_data = NULL, int use_width = 0, int use_height = 0);
	void change_texture(string name);
	void change_texture(int id);
	void change_character(FT_Bitmap bitmap, char c, bool highlight = false, int array_num = 0, bool load = false);
	static int get_height(string name);
	static int get_width(string name);
	GLubyte* get_pixel_data(string name);
	unordered_map<string, int> index_map;
	void test_save_image();
	void save_image(string name, GLubyte* buffer, int width, int height);
	void make_alias(string new_name, string old_name);
};

#endif