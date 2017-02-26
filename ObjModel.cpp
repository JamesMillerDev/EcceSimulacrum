#define _CRT_SECURE_NO_WARNINGS

#include <fstream>
#include <sstream>
#include <algorithm>
#include "ObjModel.h"
#include "GameParams.h"

using std::ifstream;
using std::istringstream;
using std::getline;

//Thanks to http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c for split()
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems, stringstream& ss, string& item) {
	ss.str(string());
	ss.clear();
	ss.str(s);
	int i = 0;
	while (std::getline(ss, item, delim)) {
		elems[i] = item;
		++i;
	}
	elems[i] = "q";
	return elems;
}

std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems(10);
	stringstream ss;
	string item;
	split(s, delim, elems, ss, item);
	return elems;
}
///////////

std::string get_file_contents(const char *filename)
{
	std::FILE *fp = std::fopen(filename, "rb");
	if (fp)
	{
		std::string contents;
		std::fseek(fp, 0, SEEK_END);
		contents.resize(std::ftell(fp));
		std::rewind(fp);
		std::fread(&contents[0], 1, contents.size(), fp);
		std::fclose(fp);
		return(contents);
	}
	throw(errno);
}

ObjModel::ObjModel(string file_name, TextureManager* _texture_manager)
{
	texture_manager = _texture_manager;
	//load_file(file_name);
	display_list = glGenLists(1);
}

//Only one object per file
//Every vertex must have tex coords and a normal
//Otherwise bad things will happen
void ObjModel::load_file(string file_name)
{
	//ifstream file_stream(file_name);
	string line;
	string contents = get_file_contents(file_name.c_str());
	stringstream file_stream(contents);
	vector<string> first_elems(6);
	vector<string> second_elems(4);
	stringstream ss;
	string for_split;
	vector<Vector3> triangle_points(3);
	vector<Vector3> quad_points(4);
	int face_index = 0;
	while (getline(file_stream, line))
	{
		istringstream string_stream(line);
		if (line[0] == 'v' && line[1] == ' ')
		{
			string_stream.seekg(2);
			float x, y, z;
			string_stream >> x >> y >> z;
			vertices.push_back(Vector3(x, y, z));
		}

		else if (line[0] == 'v' && line[1] == 't')
		{
			string_stream.seekg(3);
			float x, y;
			string_stream >> x >> y;
			tex_coords.push_back(Vector3(x, y, 0.0));
		}

		else if (line[0] == 'v' && line[1] == 'n')
		{
			string_stream.seekg(3);
			float x, y, z;
			string_stream >> x >> y >> z;
			normals.push_back(Vector3(x, y, z));
		}

		else if (line[0] == 'f')
		{
			//vector<Vector3> face_points;
			/*vector<string> points = */split(line, ' ', first_elems, ss, for_split);
			for (int i = 1; first_elems[i] != "q"; ++i)
			{
				/*vector<string> indices = */split(first_elems[i], '/', second_elems, ss, for_split);
				if (first_elems[4] == "q")
					triangle_points[i - 1] = Vector3(atoi(second_elems[0].c_str()) - 1, atoi(second_elems[1].c_str()) - 1, atoi(second_elems[2].c_str()) - 1);

				else
					quad_points[i - 1] = Vector3(atoi(second_elems[0].c_str()) - 1, atoi(second_elems[1].c_str()) - 1, atoi(second_elems[2].c_str()) - 1);
			}

			if (first_elems[4] == "q")
				faces[face_index++] = triangle_points;
				//faces.push_back(triangle_points);

			else
				faces[face_index++] = quad_points;
				//faces.push_back(quad_points);
		}

		else if (line.substr(0, 6) == "usemtl")
			texture_switch_faces.push_back(face_index);
	}

	glNewList(display_list, GL_COMPILE);
	int current_texture = -1;
	int switch_face_index = 0;
	for (int i = 0; i < faces.size(); ++i)
	{
		if (switch_face_index < texture_switch_faces.size() && i == texture_switch_faces[switch_face_index])
		{
			texture_manager->change_texture(textures[++current_texture]);
			if (textures[current_texture] == "background2.png") //hackity hack hack
				glBlendFunc(GL_ONE, GL_ONE);

			else
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			++switch_face_index;
		}

		glBegin(GL_POLYGON); //Faster to combine it to one GL_QUADS?
		for (int j = 0; j < faces[i].size(); ++j)
		{
			glTexCoord2f(tex_coords[faces[i][j].y].x, tex_coords[faces[i][j].y].y);
			glNormal3f(normals[faces[i][j].z].x, normals[faces[i][j].z].y, normals[faces[i][j].z].z);
			glVertex3f(vertices[faces[i][j].x].x, vertices[faces[i][j].x].y, vertices[faces[i][j].x].z);
		}
		glEnd();
	}
	glEndList();
}

void ObjModel::draw()
{
	glCallList(display_list);
}

void ObjModel::add_texture(string texture)
{
	textures.push_back(texture);
}

void ObjModel::reserve(int num_faces)
{
	faces.resize(num_faces);
}