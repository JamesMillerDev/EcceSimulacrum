#ifndef CAMERA_FILE_ANIMATION
#define CAMERA_FILE_ANIMATION

#include <iostream>
#include <vector>
#include "ObjModel.h"
#include "Vector3.h"

using namespace std;

struct CameraFileAnimation
{
	int frame, max_frame;
	vector<Vector3> position;
	vector<Vector3> at;
	vector<Vector3> up;
	CameraFileAnimation() {}
	CameraFileAnimation(string name)
	{
		frame = 0;
		max_frame = 0;
		ifstream file_stream(name);
		string line;
		while (getline(file_stream, line))
		{
			if (line == "")
				continue;

			max_frame++;
			vector<string> numbers = real_split(line, ' ');
			position.push_back({ (float) (atof(numbers[0].c_str()) * (512.0 / 10.0)), (float) (atof(numbers[2].c_str()) * (512.0 / 10.0)), (float) (atof(numbers[1].c_str()) * (512.0 / 10.0)) });
			at.push_back({((float) atof(numbers[3].c_str()) + position.back().x), (float) atof(numbers[5].c_str()) + position.back().y, (float) atof(numbers[4].c_str()) + position.back().z });
			up.push_back({((float) atof(numbers[6].c_str())), (float) atof(numbers[8].c_str()), (float) atof(numbers[7].c_str()) });
		}

		file_stream.close();
	}

	bool next()
	{
		frame++;
		if (frame >= max_frame)
			return false;

		return true;
	}
};

#endif