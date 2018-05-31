#ifndef SOUND_MANAGER
#define SOUND_MANAGER

#include <vector>
#include <string>
#include <map>
#include <al.h>
#include <alc.h>

using namespace std;

struct SoundManager
{
	string current_escape_sound;
	map<string, ALuint> buffers;
	map<string, vector<ALuint>> sources;
	map<string, bool> stopping;
	void load_sound(string name);
	void buffer_data_from_wav(const char *fname, ALuint buffer);
	void play_sound(string name, float reference_distance = 80.0, float source_x = 0.0, float source_y = 0.0, float source_z = 0.0, float listen_x = 0.0, float listen_y = 0.0, float listen_z = 0.0, float* orientation = 0, bool looping = false);
	void stop_sound(string name);
	void release_sources();
	void change_source_velocity(string name, float x, float y, float z);
	bool is_playing(string name);
	void smooth_stop(string name);
	void update_smooth_stop();
};

#endif