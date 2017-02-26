#ifndef OBJ_MODEL
#define OBJ_MODEL

#include <string>
#include <vector>
#include "Vector3.h"
#include "TextureManager.h"

using std::string;
using std::vector;

class ObjModel
{
private:
	vector<Vector3> vertices;
	vector<Vector3> tex_coords;
	vector<Vector3> normals;
	vector<vector<Vector3> > faces;
	vector<string> textures;
	vector<int> texture_switch_faces;
	TextureManager* texture_manager;
	GLuint display_list;

public:
	ObjModel(string file_name, TextureManager* _texture_manager);
	void draw();
	void add_texture(string texture);
	void load_file(string file_name);
	void reserve(int num_faces);
};

std::vector<std::string> split(const std::string &s, char delim);

#endif