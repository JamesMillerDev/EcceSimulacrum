#include "TextureManager.h"

map<string, int> TextureManager::height_map;
map<string, int> TextureManager::width_map;

TextureManager::TextureManager(int num_textures)
{
	current_index = 0;
	max_textures = num_textures;
	textures = (GLuint*) malloc(sizeof(GLuint) * num_textures);
	pixel_array = (unsigned char*) malloc(sizeof(unsigned char) * 200 * 200 * 4); //This is big enough right?
	glGenTextures(num_textures, textures);
	glGenTextures(128, liberation_small);
	glGenTextures(128, liberation_small_highlight);
	glGenTextures(128, liberation_big_highlight);
	glGenTextures(128, liberation_big);
	glGenTextures(128, mono_small_highlight);
	glGenTextures(128, audiowide);
	glGenTextures(128, audiowide_big);
	ilInit(); //seems like a good enough place to call this right?
	iluInit();
	ILuint image_pointer;
	ilGenImages(1, &image_pointer);
	ilBindImage(image_pointer);
}

//Apparently I need to delete all textures eventually, otherwise I'm creating memory leaks
void TextureManager::load_texture(string name, bool mips, bool retain_pixel_data, GLubyte* use_data, int use_width, int use_height)
{
	if (current_index == max_textures) //change this...?
		std::exit(1);

	ILuint width, height;
	ILubyte* pixels;
	if (!use_data)
	{
		ilLoadImage(name.c_str());
		ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
		pixels = ilGetData();
		width = ilGetInteger(IL_IMAGE_WIDTH);
		height = ilGetInteger(IL_IMAGE_HEIGHT);
	}

	else
	{
		pixels = use_data;
		width = use_width;
		height = use_height;
	}

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
	if (retain_pixel_data)
	{
		/*GLubyte* data = (GLubyte*) malloc(sizeof(GLubyte) * width * height * 4); //TODO cleanup?
		memcpy(data, pixels, sizeof(GLubyte) * width * height * 4);
		for (int j = 0; j < height; ++j)
		{
			for (int i = 0; i < width; ++i)
			{
				int data_start = j * (width) * 4 + i * 4;
				int pixels_start = (height - j - 1) * width * 4 + i * 4;
				data[data_start] = pixels[pixels_start];
				data[data_start + 1] = pixels[pixels_start + 1];
				data[data_start + 2] = pixels[pixels_start + 2];
				data[data_start + 3] = pixels[pixels_start + 3];
			}
		}

		pixel_data[name] = data;*/

		GLubyte* data = (GLubyte*)malloc(sizeof(GLubyte) * width * height * 4);
		iluFlipImage();
		memcpy(data, pixels, sizeof(GLubyte) * width * height * 4);
		iluFlipImage();
		pixel_data[name] = data;
		if (name == "print.png")
		{
			for (int k = 0; k < width * height * 4; k += 1920 * 4)
			{
				cout << (int) data[k] << " ";
				cout << (int) data[k + 1] << " ";
				cout << (int) data[k + 2] << " ";
				cout << (int) data[k + 3] << "\n";
			}
		}
	}
}

void TextureManager::change_character(FT_Bitmap bitmap, char c, bool highlight, int array_num, bool load)
{
	GLuint* char_array;
	if (array_num == 0)
		char_array = liberation_small;

	else if (array_num == 1)
		char_array = liberation_small_highlight;

	else if (array_num == 2)
		char_array = liberation_big_highlight;

	else if (array_num == 3)
		char_array = liberation_big;

	else if (array_num == 4)
		char_array = mono_small_highlight;

	else if (array_num == 5)
		char_array = audiowide_big;

	else char_array = audiowide;
	if (load)
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
					/*pixel_array[cur_pixel++] = val;
					pixel_array[cur_pixel++] = val;
					pixel_array[cur_pixel++] = val;
					pixel_array[cur_pixel++] = 255 - val;*/
					pixel_array[cur_pixel++] = 0;
					pixel_array[cur_pixel++] = 0;
					pixel_array[cur_pixel++] = 0;
					pixel_array[cur_pixel++] = 255 - val;
				}

				else
				{
					pixel_array[cur_pixel++] = 255;
					pixel_array[cur_pixel++] = 255;
					pixel_array[cur_pixel++] = 255;
					pixel_array[cur_pixel++] = 255 - val;
				}
			}
		}

		glBindTexture(GL_TEXTURE_2D, char_array[(int)c]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_array);
		//glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, width, height, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, bitmap.buffer); //apparently glTexImage2D isn't needed if we call mipmaps()
		//gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixel_array);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);//0x812F);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);//0x812F);
		/*index_map[str] = current_index;
		width_map[str] = width;
		height_map[str] = height;
		current_index++;*/
	}

	glBindTexture(GL_TEXTURE_2D, char_array[(int)c]);
}

void TextureManager::make_alias(string new_name, string old_name)
{
	index_map[new_name] = index_map[old_name];
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

GLubyte* TextureManager::get_pixel_data(string name)
{
	return pixel_data[name];
}

void TextureManager::test_save_image()
{
	ilEnable(IL_FILE_OVERWRITE);
	ilLoadImage("testimage.png");
	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	ILubyte* pixels = ilGetData();
	for (int i = 0; i < 50 * 4; i += 4)
	{
		pixels[i] = 255;
		pixels[i + 1] = 0;
		pixels[i + 2] = 0;
		pixels[i + 3] = 255;
	}

	ilSave(IL_PNG, "testimage.png");
}

void TextureManager::save_image(string name, GLubyte* buffer, int width, int height)
{
	ilEnable(IL_FILE_OVERWRITE);
	ilLoadImage(name.c_str());
	ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	ILubyte* pixels = ilGetData();
	memcpy(pixels, buffer, sizeof(GLubyte) * width * height * 4);
	iluFlipImage();
	ilSave(IL_PNG, name.c_str());
}

TextureManager::~TextureManager()
{
	glDeleteTextures(max_textures, textures);
	free(textures);
	free(pixel_array);
}