#include "TextureManager.h"

map<string, int> TextureManager::height_map;
map<string, int> TextureManager::width_map;

TextureManager::TextureManager(int num_textures)
{
	current_index = 0;
	max_textures = num_textures;
	textures = (GLuint*) malloc(sizeof(GLuint) * num_textures);
	char_texture = (GLuint*) malloc(sizeof(GLuint)); //for drawing characters from fonts
	pixel_array = (unsigned char*) malloc(sizeof(unsigned char) * 200 * 200 * 4); //This is big enough right?
	glGenTextures(num_textures, textures);
	glGenTextures(1, char_texture);
	ilInit(); //seems like a good enough place to call this right?
	ILuint image_pointer;
	ilGenImages(1, &image_pointer);
	ilBindImage(image_pointer);
}

//Apparently I need to delete all textures eventually, otherwise I'm creating memory leaks
void TextureManager::load_texture(string name, bool mips)
{
	if (current_index == max_textures) //change this...?
		std::exit(1);

	ilLoadImage(name.c_str());
	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	ILubyte* pixels = ilGetData();
	ILuint width, height;
	width = ilGetInteger(IL_IMAGE_WIDTH);
	height = ilGetInteger(IL_IMAGE_HEIGHT);
	glBindTexture(GL_TEXTURE_2D, textures[current_index]);
	if (mips)
	{
		gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}

	else
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//0x812F);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//0x812F);

	GLfloat aniso;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso); //need to check if aniso is enabled?

	index_map[name] = current_index;
	height_map[name] = height;
	width_map[name] = width;
	current_index++;
}

void TextureManager::change_character(FT_Bitmap bitmap, string c, bool highlight)
{
	string str = c;
	if (highlight)
		str += "highlight";

	if (index_map.find(str) == index_map.end())
	{
		int width = bitmap.width;
		int height = bitmap.rows;
		int cur_pixel = 0;
		for (int row = height - 1; row >= 0; --row)
		{
			for (int col = 0; col < width; ++col)
			{
				unsigned char val = 255 - bitmap.buffer[row*width + col];
				if (!highlight)
				{
					pixel_array[cur_pixel++] = val;
					pixel_array[cur_pixel++] = val;
					pixel_array[cur_pixel++] = val;
					pixel_array[cur_pixel++] = 255 - val;
				}

				else
				{
					pixel_array[cur_pixel++] = 255 - val;
					pixel_array[cur_pixel++] = 255 - val;
					pixel_array[cur_pixel++] = 255 - val;
					pixel_array[cur_pixel++] = 255 - val;
				}
			}
		}

		glBindTexture(GL_TEXTURE_2D, textures[current_index]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_array);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, bitmap.buffer); //apparently glTexImage2D isn't needed if we call mipmaps()
		//gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixel_array);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//0x812F);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//0x812F);
		index_map[str] = current_index;
		current_index++;
	}

	glBindTexture(GL_TEXTURE_2D, textures[index_map[str]]);
}

void TextureManager::change_texture(string name)
{
	glBindTexture(GL_TEXTURE_2D, textures[index_map[name]]);
}

void TextureManager::change_texture(int id)
{
	glBindTexture(GL_TEXTURE_2D, id);
}

int TextureManager::get_height(string name)
{
	return height_map[name];
}

int TextureManager::get_width(string name)
{
	return width_map[name];
}

TextureManager::~TextureManager()
{
	glDeleteTextures(max_textures, textures);
	free(textures);
	free(char_texture);
	free(pixel_array);
}