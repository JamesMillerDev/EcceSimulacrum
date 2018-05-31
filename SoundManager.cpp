#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <algorithm>
#include <iostream>
#include "SoundManager.h"

#define _CRT_SECURE_NO_WARNINGS

void SoundManager::buffer_data_from_wav(const char *fname, ALuint buffer)
{
	FILE *fp;
	fp = fopen(fname, "rb");
	short format_tag, channels, block_align, bits_per_sample;
	DWORD format_length, sample_rate, avg_bytes_sec, data_size, i;
	char id[5], *sound_buffer;
	sound_buffer = 0;
	if (fp)
	{
		char size[4];
		fread(id, sizeof(BYTE), 4, fp);
		id[4] = 0;
		if (!strcmp(id, "RIFF"))
		{
			fread(size, sizeof(DWORD), 1, fp);
			fread(id, sizeof(BYTE), 4, fp);
			if (!strcmp(id, "WAVE"))
			{
				fread(id, sizeof(BYTE), 4, fp);
				fread(&format_length, sizeof(DWORD), 1, fp);
				fread(&format_tag, sizeof(short), 1, fp);
				fread(&channels, sizeof(short), 1, fp);
				fread(&sample_rate, sizeof(DWORD), 1, fp);
				fread(&avg_bytes_sec, sizeof(DWORD), 1, fp);
				fread(&block_align, sizeof(short), 1, fp);
				fread(&bits_per_sample, sizeof(short), 1, fp);
				fseek(fp, format_length - 16, SEEK_CUR);
				fread(id, sizeof(BYTE), 4, fp);
				fread(&data_size, sizeof(DWORD), 1, fp);
				sound_buffer = (char *)malloc(sizeof(BYTE) * data_size);
				fread(sound_buffer, sizeof(BYTE), data_size, fp);
			}
		}
	}

	ALenum format;
	if (bits_per_sample == 16)
	{
		if (channels > 1)
			format = AL_FORMAT_STEREO16;

		else format = AL_FORMAT_MONO16;
	}

	else
	{
		if (channels > 1)
			format = AL_FORMAT_STEREO8;

		else format = AL_FORMAT_MONO8;
	}

	alBufferData(buffer, format, sound_buffer, data_size, sample_rate);
}

void SoundManager::load_sound(string name)
{
	ALuint new_buffer;
	alGenBuffers((ALuint)1, &new_buffer);
	buffer_data_from_wav(name.c_str(), new_buffer);
	buffers[name] = new_buffer;
}

void SoundManager::play_sound(string name, float reference_distance, float source_x, float source_y, float source_z, float listen_x, float listen_y, float listen_z, float* orientation, bool looping)
{
	ALuint new_source;
	alGenSources((ALuint)1, &new_source);
	alSourcef(new_source, AL_PITCH, 1);
	alSourcef(new_source, AL_GAIN, 1.0);
	alSource3f(new_source, AL_POSITION, source_x, source_y, source_z);
	alSource3f(new_source, AL_VELOCITY, 0, 0, 0);
	alSourcei(new_source, AL_LOOPING, looping ? AL_TRUE : AL_FALSE);
	alSourcef(new_source, AL_REFERENCE_DISTANCE, reference_distance);
	alSourcei(new_source, AL_BUFFER, buffers[name]);
	sources[name].push_back(new_source);
	alListener3f(AL_POSITION, listen_x, listen_y, listen_z);
	if (orientation == 0)
	{
		ALfloat default[] = { 0, 0, 1, 0, 1, 0 };
		alListenerfv(AL_ORIENTATION, default);
	}

	else alListenerfv(AL_ORIENTATION, orientation);
	alSourcePlay(new_source);
}

void SoundManager::stop_sound(string name)
{
	for (int i = 0; i < sources[name].size(); ++i)
	{
		alSourcei(sources[name][i], AL_GAIN, 0);
		alSourceStop(sources[name][i]);
	}
}

void SoundManager::smooth_stop(string name)
{
	if (sources[name].size() != 0)
		stopping[name] = true;
}

void SoundManager::update_smooth_stop()
{
	for (auto const& sound : stopping)
	{
		if (sound.second)
		{
			if (sources[sound.first].size() == 0)
				continue;

			float gain;
			alGetSourcef(sources[sound.first][0], AL_GAIN, &gain);
			if (gain <= 0.1)
			{
				stopping[sound.first] = false;
				alSourceStop(sources[sound.first][0]);
			}

			else alSourcef(sources[sound.first][0], AL_GAIN, gain - 0.1);
		}
	}
}

void SoundManager::release_sources()
{
	vector<ALuint> to_delete;
	for (auto &pair : sources)
	{
		pair.second.erase(remove_if(pair.second.begin(), pair.second.end(), [&to_delete](ALuint source)
		{
			ALenum state;
			alGetSourcei(source, AL_SOURCE_STATE, &state);
			if (state != AL_PLAYING)
			{
				to_delete.push_back(source);
				return true;
			}

			return false;
		}), pair.second.end());
	}

	for (int i = 0; i < to_delete.size(); ++i)
		alDeleteSources(1, &to_delete[i]);
}

void SoundManager::change_source_velocity(string name, float x, float y, float z)
{
	for (int i = 0; i < sources[name].size(); ++i)
		alSource3f(sources[name][i], AL_VELOCITY, x, y, z);
}

bool SoundManager::is_playing(string name)
{
	return sources[name].size() != 0;
}