#define _CRT_SECURE_NO_WARNINGS

//TODO check every srand to make sure no time_t
//TODO changed silver to black, lightbulb metal part is black now

#include <ctime>
#include <time.h>
#include <process.h>
#include <sstream>
#include <memory>
#include "TextureManager.h"
#include "Camera.h"
#include "ObjModel.h"
#include "Computer.h"
#include "fonts.h"
#include "keymappings.h"
#include "SoundManager.h"
#include "binaryio.h"
#include "PauseMenu.h"

struct shadow_rect
{
	float x1, y1, x2, y2;
	float alpha;
	shadow_rect(float _x1, float _x2, float _y1, float _y2, float _alpha) : x1(_x1), x2(_x2), y1(_y1), y2(_y2), alpha(_alpha) {}
};

enum GameState {STATE_AWAKENING, STATE_NO_COMPUTER, STATE_WALKING, STATE_COMPUTER, STATE_PEEING, STATE_GETTING_UP, STATE_FLYING, STATE_LOADING};
GameState game_state = STATE_LOADING;
std::unique_ptr<Camera> camera;
std::unique_ptr<TextureManager> texture_manager;
std::unique_ptr<SoundManager> sound_manager;
std::unique_ptr<ObjModel> desk;
std::unique_ptr<ObjModel> bulb;
std::unique_ptr<ObjModel> monitor;
std::unique_ptr<Computer> computer;
std::unique_ptr<PauseMenu> pause_menu;

int elapsed_eyelid_time = 0;
int timebase = 0;
std::vector<shadow_rect> rects;
int continue_state = -1;
bool do_pause_menu = false;

float eyelid_offset = 0.0;
float eyelid_velocity = 5.0;
float eyelid_accel = 0.0;
float look_vector_angle = PI / 2;
float look_vector_velocity = 0.0004;
int stage = 0;
float eyelid_wait_frames = 0;

const char* shader = "void main() {if (gl_FragCoord[1] < 200) gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0); else gl_FragColor = gl_Color;}";
const char* vert_shader = "void main() {gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;}";

//TEMPORARY
time_t start, end_t;
float fps = 0;
int new_frames = 0;
float low_y = 0.0;
float high_y = 1.0;
//TEMPORARY

////// parameters for flickering the lights
bool lights_flickering = false;
float current_intensity = 4.5;
float light_delta = -0.2;

bool draw_computer = true;
float current_light_x = 0.0;
float current_light_y = WALL_HEIGHT - 81.0;
float current_light_z = 0.0;
vector<Vector3> computer_setups;
int computer_setup_index = 0;
bool counting_down_to_chaos = false;
int chaos_frames = 0;
bool chaos_animation = false;
bool blue_light = false;
bool counting_to_blue_light = false;
int blue_frame_index = 1;

float loading_percentage = 0.0;
int loading_stage = 0;
void load_everything();
void reset_to_title();

float wasd_alpha = 0.0;
float wasd_alpha_inc = 0.02;

void write_binary(string str, ofstream& out)
{
	int len = str.size();
	out.write((const char*)&len, sizeof(int));
	out.write(str.c_str(), len * sizeof(char));
}

void write_binary(File file, ofstream& out)
{
	write_binary(file.name, out);
	write_binary(file.type, out);
	write_binary(file.xsize, out);
	write_binary(file.ysize, out);
	out.write((const char*)file.image, sizeof(GLubyte) * file.xsize * file.ysize * 4);
}

string read_binary(tag<string>, ifstream& in)
{
	int len;
	in.read((char*)&len, sizeof(int));
	vector<char> temp(len);
	if (len != 0)
		in.read(&temp[0], len * sizeof(char));

	return string(temp.begin(), temp.end());
}

File read_binary(tag<File>, ifstream& in)
{
	File file;
	file.image = NULL;
	file.name = read_binary<string>(in);
	file.type = read_binary<int>(in);
	file.xsize = read_binary<int>(in);
	file.ysize = read_binary<int>(in);
	if (file.xsize * file.ysize != 0)
	{
		file.image = (GLubyte*)malloc(sizeof(GLubyte) * file.xsize * file.ysize * 4);
		in.read((char*)file.image, sizeof(GLubyte) * file.xsize * file.ysize * 4);
	}

	return file;
}

void save_game(string filename)
{
	string folder = filename + "p";
	CreateDirectory(folder.c_str(), NULL);
	ofstream out(filename, ios_base::binary);
	std::time_t tm = std::time(nullptr);
	char mbstr[100];
	std::strftime(mbstr, sizeof(mbstr), "%F %T", std::localtime(&tm));
	write_binary(string(mbstr), out);
	write_binary(computer->elapsed_time, out);
	out.write((const char*)(&(computer->submitted_poll_of_the_day)), sizeof(char));
	out.write((const char*)(&(computer->no_rosecolored1)), sizeof(char));
	out.write((const char*)(&(computer->visited_elsewhere_after_rosecolored1)), sizeof(char));
	out.write((const char*)(&(computer->visited_elsewhere_before_rosecolored1)), sizeof(char));
	out.write((const char*)(&(computer->no_rosecolored2)), sizeof(char));
	out.write((const char*)(&(computer->looked_for_spider)), sizeof(char));
	out.write((const char*)(&(computer->missed_spider)), sizeof(char));
	out.write((const char*)(&(computer->no_rosecolored_missed_spider)), sizeof(char));
	out.write((const char*)(&(computer->no_rosecolored3)), sizeof(char));
	out.write((const char*)(&(computer->started_crashing_browser)), sizeof(char));
	out.write((const char*)(&(computer->crashed_browser)), sizeof(char));
	out.write((const char*)(&(computer->update_in_progress)), sizeof(char));
	out.write((const char*)(&(computer->recovered_browser)), sizeof(char));
	out.write((const char*)(&(computer->killed_process)), sizeof(char));
	out.write((const char*)(&(computer->tried_to_view_encrypted_folder)), sizeof(char));
	out.write((const char*)(&(computer->no_rosecolored4)), sizeof(char));
	out.write((const char*)(&(computer->reached_help_center)), sizeof(char));
	out.write((const char*)(&(computer->reached_psych_article)), sizeof(char));
	out.write((const char*)(&(computer->no_rosecolored5)), sizeof(char));
	out.write((const char*)(&(computer->entered_contest)), sizeof(char));
	out.write((const char*)(&(computer->entered_right_email_address)), sizeof(char));
	out.write((const char*)(&(computer->no_centerracom_trial_link)), sizeof(char));
	out.write((const char*)(&(computer->ran_trial)), sizeof(char));
	out.write((const char*)(&(computer->no_centerracom_survey_link)), sizeof(char));
	out.write((const char*)(&(computer->decryption_broken)), sizeof(char));
	out.write((const char*)(&(computer->the_end)), sizeof(char));
	out.write((const char*)(&(computer->answered_search_question)), sizeof(char));
	out.write((const char*)(&(computer->reached_second_day)), sizeof(char));
	out.write((const char*)(&(computer->reached_psychologist)), sizeof(char));
	out.write((const char*)(&(computer->contest_finished)), sizeof(char));
	out.write((const char*)(&(computer->no_contest_finished_email)), sizeof(char));
	out.write((const char*)(&(computer->no_rfh3)), sizeof(char));
	out.write((const char*)(&(computer->showed_walter)), sizeof(char));
	out.write((const char*)(&(computer->reached_space_game)), sizeof(char));
	out.write((const char*)(&(computer->beat_space_game)), sizeof(char));
	out.write((const char*)(&(computer->no_rfh7)), sizeof(char));
	out.write((const char*)(&(computer->day_number)), sizeof(int));
	write_binary(computer->emails, out);
	write_binary(computer->whitelist, out);
	write_binary(computer->websites, out);
	write_binary(computer->sequences, out);
	write_binary(computer->link_overrides, out);
	write_binary(computer->read_email, out);
	write_binary(computer->visited_site, out);
	write_binary(computer->extra_apps, out);
	write_binary(computer->uploaded_image, out);
	write_binary(computer->score_table, out);
	if (game_state == STATE_WALKING)
		write_binary(0, out);

	else write_binary(1, out);
	out.close();
}

void load_game(string filename)
{
	ifstream in(filename, ios_base::binary);
	string tm = read_binary<string>(in);
	computer->elapsed_time = read_binary<int>(in);
	in.read((char*)(&(computer->submitted_poll_of_the_day)), sizeof(char));
	in.read((char*)(&(computer->no_rosecolored1)), sizeof(char));
	in.read((char*)(&(computer->visited_elsewhere_after_rosecolored1)), sizeof(char));
	in.read((char*)(&(computer->visited_elsewhere_before_rosecolored1)), sizeof(char));
	in.read((char*)(&(computer->no_rosecolored2)), sizeof(char));
	in.read((char*)(&(computer->looked_for_spider)), sizeof(char));
	in.read((char*)(&(computer->missed_spider)), sizeof(char));
	in.read((char*)(&(computer->no_rosecolored_missed_spider)), sizeof(char));
	in.read((char*)(&(computer->no_rosecolored3)), sizeof(char));
	in.read((char*)(&(computer->started_crashing_browser)), sizeof(char));
	in.read((char*)(&(computer->crashed_browser)), sizeof(char));
	in.read((char*)(&(computer->update_in_progress)), sizeof(char));
	in.read((char*)(&(computer->recovered_browser)), sizeof(char));
	in.read((char*)(&(computer->killed_process)), sizeof(char));
	in.read((char*)(&(computer->tried_to_view_encrypted_folder)), sizeof(char));
	in.read((char*)(&(computer->no_rosecolored4)), sizeof(char));
	in.read((char*)(&(computer->reached_help_center)), sizeof(char));
	in.read((char*)(&(computer->reached_psych_article)), sizeof(char));
	in.read((char*)(&(computer->no_rosecolored5)), sizeof(char));
	in.read((char*)(&(computer->entered_contest)), sizeof(char));
	in.read((char*)(&(computer->entered_right_email_address)), sizeof(char));
	in.read((char*)(&(computer->no_centerracom_trial_link)), sizeof(char));
	in.read((char*)(&(computer->ran_trial)), sizeof(char));
	in.read((char*)(&(computer->no_centerracom_survey_link)), sizeof(char));
	in.read((char*)(&(computer->decryption_broken)), sizeof(char));
	in.read((char*)(&(computer->the_end)), sizeof(char));
	in.read((char*)(&(computer->answered_search_question)), sizeof(char));
	in.read((char*)(&(computer->reached_second_day)), sizeof(char));
	in.read((char*)(&(computer->reached_psychologist)), sizeof(char));
	in.read((char*)(&(computer->contest_finished)), sizeof(char));
	in.read((char*)(&(computer->no_contest_finished_email)), sizeof(char));
	in.read((char*)(&(computer->no_rfh3)), sizeof(char));
	in.read((char*)(&(computer->showed_walter)), sizeof(char));
	in.read((char*)(&(computer->reached_space_game)), sizeof(char));
	in.read((char*)(&(computer->beat_space_game)), sizeof(char));
	in.read((char*)(&(computer->no_rfh7)), sizeof(char));
	in.read((char*)(&(computer->day_number)), sizeof(int));
	computer->emails = read_binary<vector<string>>(in);
	computer->whitelist = read_binary<vector<string>>(in);
	computer->websites = read_binary<map<string, vector<string>>>(in);
	computer->sequences = read_binary<map<string, vector<string>>>(in);
	computer->link_overrides = read_binary<map<string, map<int, string>>>(in);
	computer->read_email = read_binary<map<string, bool>>(in);
	computer->visited_site = read_binary<map<string, bool>>(in);
	computer->extra_apps = read_binary<vector<pair<string, Application>>>(in);
	computer->uploaded_image = read_binary<File>(in);
	computer->score_table = read_binary<vector<pair<string, int>>>(in);
	int state = read_binary<int>(in);
	if (state == 0)
	{
		continue_state = 0;
		camera->camera_y = INITIAL_WALKING_Y;
		camera->look_x = 0.0;
		camera->look_y = 1.0;
		camera->look_z = 0.0;
		camera->current_vert_angle = PI / 2.0;
	}

	else if (state == 1)
	{
		continue_state = 1;
		camera->camera_x = 290.0;
		camera->camera_y = INITIAL_WALKING_Y;
		camera->camera_z = 0.0;
		camera->look_x = 0.0;
		camera->look_y = 1.0;
		camera->look_z = 0.0;
		camera->current_vert_angle = PI / 2.0;
		camera->current_horiz_angle = PI / 2.0;
	}

	camera->computer_planes();
	camera->do_collision = true;
	camera->computer_present = true;
	in.close();
}

void buffer_data_from_wav(char *fname, ALuint buffer)
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
	free(sound_buffer);
}

void init_sound()
{
	alGetError();
	ALCdevice *device;
	device = alcOpenDevice("Generic Software");
	ALCcontext *context;
	context = alcCreateContext(device, NULL);
	alcMakeContextCurrent(context);
	/*alGenSources((ALuint)1, &lightbulb_source);
	alGenSources((ALuint)1, &footsteps_source);
	alGenSources((ALuint)1, &bomb_source);
	alGenSources((ALuint)1, &title_source);
	alGenSources((ALuint)1, &main_source);
	alGenSources((ALuint)1, &game_over_source);
	alGenSources((ALuint)1, &laser_source);
	alGenSources((ALuint)1, &sounds_source);
	alSourcef(lightbulb_source, AL_PITCH, 1);
	alSourcef(lightbulb_source, AL_GAIN, 1);
	alSource3f(lightbulb_source, AL_POSITION, 0, WALL_HEIGHT - 9, 0);
	alSource3f(lightbulb_source, AL_VELOCITY, 0, 0, 0);
	alSourcei(lightbulb_source, AL_LOOPING, AL_FALSE);
	alSourcef(lightbulb_source, AL_REFERENCE_DISTANCE, 192.0);
	alSourcef(footsteps_source, AL_PITCH, 1);
	alSourcef(footsteps_source, AL_GAIN, 1);
	alSourcei(footsteps_source, AL_LOOPING, AL_FALSE);
	alSourcef(footsteps_source, AL_REFERENCE_DISTANCE, 80.0);
	alSourcei(bomb_source, AL_PITCH, 1);
	alSourcei(bomb_source, AL_GAIN, 1);
	alSourcei(bomb_source, AL_LOOPING, AL_FALSE);
	alSourcef(bomb_source, AL_REFERENCE_DISTANCE, 80.0);
	alSourcei(title_source, AL_PITCH, 1);
	alSourcei(title_source, AL_GAIN, 1);
	alSourcei(title_source, AL_LOOPING, AL_TRUE);
	alSourcef(title_source, AL_REFERENCE_DISTANCE, 80.0);
	alSourcei(main_source, AL_PITCH, 1);
	alSourcei(main_source, AL_GAIN, 1);
	alSourcei(main_source, AL_LOOPING, AL_TRUE);
	alSourcef(main_source, AL_REFERENCE_DISTANCE, 80.0);
	alSourcei(game_over_source, AL_PITCH, 1);
	alSourcei(game_over_source, AL_GAIN, 1);
	alSourcei(game_over_source, AL_LOOPING, AL_FALSE);
	alSourcef(game_over_source, AL_REFERENCE_DISTANCE, 80.0);
	alSourcei(laser_source, AL_PITCH, 1);
	alSourcei(laser_source, AL_GAIN, 1);
	alSourcei(laser_source, AL_LOOPING, AL_FALSE);
	alSourcef(laser_source, AL_REFERENCE_DISTANCE, 80.0);
	alSourcei(sounds_source, AL_PITCH, 1);
	alSourcei(sounds_source, AL_GAIN, 1);
	alSourcei(sounds_source, AL_LOOPING, AL_FALSE);
	alSourcef(sounds_source, AL_REFERENCE_DISTANCE, 80.0);*/
	sound_manager->load_sound("light.wav");
	sound_manager->load_sound("step.wav");
	sound_manager->load_sound("bomb.wav");
	sound_manager->load_sound("title.wav");
	sound_manager->load_sound("main.wav");
	sound_manager->load_sound("gameover.wav");
	sound_manager->load_sound("laser.wav");
	sound_manager->load_sound("thesounds.wav");
	sound_manager->load_sound("thesounds2.wav");
	sound_manager->load_sound("takeoff.wav");
	sound_manager->load_sound("beep.wav");
	sound_manager->load_sound("thunk.wav");
	sound_manager->load_sound("rustle1.wav");
	sound_manager->load_sound("rustle2.wav");
	sound_manager->load_sound("boxbeep.wav");
	sound_manager->load_sound("buzzer.wav");
	sound_manager->load_sound("ding.wav");
	sound_manager->load_sound("startup.wav");
	sound_manager->load_sound("startupbeep.wav");
	sound_manager->load_sound("startupmusic.wav");
	sound_manager->load_sound("ambient.wav");
}

void flicker_lights()
{
	if (current_intensity == 4.5)
		sound_manager->play_sound("light.wav");
	
	current_intensity += light_delta;
	if (current_intensity <= 1.5)
		light_delta *= -1.0;

	else if (current_intensity >= 4.5)
	{
		light_delta *= -1.0;
		lights_flickering = false;
	}
}

//TODO: What if someone leaves the game running for 24 days?
void animate_eyelids()
{
	if (timebase != 0)
		elapsed_eyelid_time = glutGet(GLUT_ELAPSED_TIME) - timebase;
	
	if (elapsed_eyelid_time == 0)
	{
		//eyelid_cycles = 0;
		timebase = glutGet(GLUT_ELAPSED_TIME);
		//int width = glutGet(GLUT_WINDOW_WIDTH);
		//for (int i = 0; i <= glutGet(GLUT_WINDOW_HEIGHT) + 3; ++i)
		//	rects.push_back(shadow_rect(0, width, i, i+3, 1.0));
	}

	//if (elapsed_eyelid_time >= 3000)
	{
	switch (stage)
	{
	case 0:
		if (eyelid_offset >= 70)
		{
			eyelid_accel = -0.07;
			stage++;
		}
		break;

	case 1:
		if (eyelid_velocity <= -0.02)
		{
			eyelid_accel = 0.0;
			lights_flickering = true;
			stage++;
		}
		break;

	case 2:
		if (eyelid_offset <= 250.0)
		{
			lights_flickering = true;
			stage++;
		}

	case 3:
		if (eyelid_offset <= 235.0)
		{
			eyelid_velocity = -0.5;
			lights_flickering = true;
			stage++;
		}
		break;

	case 4:
		if (eyelid_offset <= 0.0)
		{
			//eyelid_velocity = 0.0;
			eyelid_velocity = 5.0;
			eyelid_accel = 0.0;
			game_state = STATE_GETTING_UP;
			if (!DEBUG)
				camera->start_file_animation("anim2");

			camera->look_x = 0.0;
			camera->look_y = 1.0;
			/*camera->look_x = 0.0;
			camera->look_y = 0.0;
			camera->look_z = -1.0;*/
			camera->current_horiz_angle -= 1;
			stage++;
			return;
		}
		break;

	case 5:
		if (eyelid_offset >= 235.0)
		{
			eyelid_velocity = 0.0;
			stage++;
		}
		break;

	case 6:
		eyelid_wait_frames++;
		if (eyelid_wait_frames > 6.5 * 60)
		{
			eyelid_velocity = -40;
			stage++;
		}
		break;

	case 7:
		if (eyelid_offset <= 0)
		{
			eyelid_velocity = 40;
			stage++;
		}
		break;
	}
	

	eyelid_velocity += eyelid_accel;
	eyelid_offset += eyelid_velocity;
	look_vector_angle += look_vector_velocity;
	if (look_vector_angle < .45*PI || look_vector_angle > .55*PI)
		look_vector_velocity *= -1.0;
	if (stage <= 4)
		camera->tilt_look_vector(look_vector_angle);
	}
}

/*void init_shaders()
{
	//GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint vert = glCreateShader(GL_VERTEX_SHADER);
	//glShaderSource(frag, 1, &shader, NULL);
	glShaderSource(vert, 1, &vert_shader, NULL);
	//glCompileShader(frag);
	glCompileShader(vert);
	GLuint program = glCreateProgram();
	glAttachShader(program, vert);
	//glAttachShader(program, frag);
	glLinkProgram(program);
	glUseProgram(program);
}*/

void init_texture()
{
	texture_manager->load_texture("tiles.bmp", true); //Not loading this looks surprisingly good right now, I think it uses edges.bmp instead of tiles.bmp
	texture_manager->load_texture("edges.bmp", true);
	texture_manager->load_texture("font.bmp", true);
	texture_manager->load_texture("powersymbol.png", false, true);
	texture_manager->load_texture("powersymbol1.png");
	texture_manager->load_texture("powersymbol2.png");
	texture_manager->load_texture("powersymbol3.png");
	texture_manager->load_texture("powersymbol4.png");
	texture_manager->load_texture("powersymbol5.png");
	texture_manager->load_texture("powersymbol6.png");
	texture_manager->load_texture("background.png");
	texture_manager->load_texture("bar.png", true);
	texture_manager->load_texture("test.png", true);
	texture_manager->load_texture("background2.png", true);
	texture_manager->load_texture("silver.png");
	texture_manager->load_texture("black.png", true);
	texture_manager->load_texture("light.png", true);
	texture_manager->load_texture("textfield.png");
	texture_manager->load_texture("xbutton.png");
	texture_manager->load_texture("search.png", false, true);
	texture_manager->load_texture("browserplan.png");
	texture_manager->load_texture("test2.png");
	texture_manager->load_texture("white.png", true);
    texture_manager->load_texture("icon_internet.png");
	texture_manager->load_texture("icon_internet_text.png");
    texture_manager->load_texture("arrow.png", true);
    texture_manager->load_texture("arrow2.png", true);
    texture_manager->load_texture("minimize.png");
    texture_manager->load_texture("back.png");
    texture_manager->load_texture("rfh1.png");
	texture_manager->load_texture("icon_folder.png");
	texture_manager->load_texture("icon_console.png", true);
	texture_manager->load_texture("beige.png");
	texture_manager->load_texture("beige2.png");
	texture_manager->load_texture("beigeold.png");
	texture_manager->load_texture("bigbeige.png");
	texture_manager->load_texture("ok.png");
	texture_manager->load_texture("rfh1bold.png");
	texture_manager->load_texture("rfh1regular.png");
	texture_manager->load_texture("rfh2bold.png");
	texture_manager->load_texture("rfh2regular.png");
	texture_manager->load_texture("rfh2.png");
	texture_manager->load_texture("nendablank.png");
	texture_manager->load_texture("tablet.png");
	texture_manager->load_texture("reddit.png");
	texture_manager->load_texture("deliverfailurebold.png");
	texture_manager->load_texture("deliverfailureregular.png");
	texture_manager->load_texture("deliverfailure.png");
	texture_manager->load_texture("forumemail1bold.png");
	texture_manager->load_texture("forumemail1regular.png");
	texture_manager->load_texture("forumemail1.png");
	texture_manager->load_texture("forumemail2bold.png");
	texture_manager->load_texture("forumemail2regular.png");
	texture_manager->load_texture("forumemail2.png");
	texture_manager->load_texture("usefulbookmark.png");
	texture_manager->load_texture("forward.png");
	texture_manager->load_texture("nendabookmark.png");
	texture_manager->load_texture("socnewsbookmark.png");
	texture_manager->load_texture("errorpage.png");
	texture_manager->load_texture("rosecolored1bold.png");
	texture_manager->load_texture("rosecolored1regular.png");
	texture_manager->load_texture("rosecolored1.png");
	texture_manager->load_texture("rosecolored2bold.png");
	texture_manager->load_texture("rosecolored2regular.png");
	texture_manager->load_texture("rosecolored2.png");
	texture_manager->load_texture("rosecolored3bold.png");
	texture_manager->load_texture("rosecolored3regular.png");
	texture_manager->load_texture("rosecolored3.png");
	texture_manager->load_texture("lock.png", true);
	texture_manager->load_texture("rosecolored4bold.png");
	texture_manager->load_texture("rosecolored4regular.png");
	texture_manager->load_texture("rosecolored4.png");
	texture_manager->load_texture("red.png", true);
	texture_manager->load_texture("orange.png", true);
	texture_manager->load_texture("yellow.png", true);
	texture_manager->load_texture("green.png", true);
	texture_manager->load_texture("blue.png", true);
	texture_manager->load_texture("indigo.png", true);
	texture_manager->load_texture("violet.png", true);
	texture_manager->load_texture("icon_paint.png", true);
	texture_manager->load_texture("bigblue.png", true);
	texture_manager->load_texture("bigred.png", true);
	texture_manager->load_texture("biggreen.png", true);
	texture_manager->load_texture("bigyellow.png", true);
	texture_manager->load_texture("searchresults.png");
	texture_manager->load_texture("rosecoloredmissedspider.png");
	texture_manager->load_texture("rosecoloredmissedspiderbold.png");
	texture_manager->load_texture("rosecoloredmissedspiderregular.png");
	texture_manager->load_texture("go.png");
	texture_manager->load_texture("history.png");
	texture_manager->load_texture("scrollbar.png");
	texture_manager->load_texture("ainbookmark.png");
	texture_manager->load_texture("searchbookmark.png");
	texture_manager->load_texture("ain1.png");
	texture_manager->load_texture("e1.png");
	texture_manager->load_texture("psych.png");
	texture_manager->load_texture("rosecolored5bold.png");
	texture_manager->load_texture("rosecolored5regular.png");
	texture_manager->load_texture("rosecolored5.png");
	texture_manager->load_texture("spaceship.png");
	texture_manager->load_texture("thankyouforuploading.png");
	texture_manager->load_texture("centerracomhomepage.png");
	texture_manager->load_texture("bannerad.png");
	texture_manager->load_texture("centerracomtrial.png");
	texture_manager->load_texture("centerracomthankyou.png");
	texture_manager->load_texture("triallinkbold.png");
	texture_manager->load_texture("triallinkregular.png");
	texture_manager->load_texture("triallink.png");
	texture_manager->load_texture("icon_video.png");
	texture_manager->load_texture("surveylinkbold.png");
	texture_manager->load_texture("surveylinkregular.png");
	texture_manager->load_texture("surveylink.png");
	texture_manager->load_texture("icon_decrypt.png");
	texture_manager->load_texture("maindown0.png", true);
	texture_manager->load_texture("maindown1.png", true);
	texture_manager->load_texture("treesbackground.png");
	texture_manager->load_texture("river.png");
	texture_manager->load_texture("river1.png", true);
	texture_manager->load_texture("river2.png", true);
	texture_manager->load_texture("mainup0.png", true);
	texture_manager->load_texture("mainup1.png", true);
	texture_manager->load_texture("mainright0.png", true);
	texture_manager->load_texture("mainright1.png", true);
	texture_manager->load_texture("mainleft0.png", true);
	texture_manager->load_texture("mainleft1.png", true);
	texture_manager->load_texture("star.png", true);
	texture_manager->load_texture("bullet.png", true);
	texture_manager->load_texture("ship.png", true);
	texture_manager->load_texture("alien.png", true);
	texture_manager->load_texture("alienbullet.png", true);
	texture_manager->load_texture("explosion1.png", true);
	texture_manager->load_texture("explosion2.png", true);
	texture_manager->load_texture("shadow.png", true);
	texture_manager->load_texture("sheen.png", true);
	texture_manager->load_texture("sheen2.png", true);
	texture_manager->load_texture("cap1.png");
	texture_manager->load_texture("cap2.png");
	texture_manager->load_texture("arrowthing.png");
	texture_manager->load_texture("smallarrow.png", true);
	texture_manager->load_texture("circle.png", true);
	texture_manager->load_texture("searchbar.png");
	texture_manager->load_texture("searchbutton.png");
	texture_manager->load_texture("helphome.png");
	texture_manager->load_texture("chatbutton.png");
	texture_manager->load_texture("forumhome.png");
	texture_manager->load_texture("header.png");
	texture_manager->load_texture("ellipse.png");
	texture_manager->load_texture("fill.png");
	texture_manager->load_texture("filledrectangle.png");
	texture_manager->load_texture("line.png");
	texture_manager->load_texture("paintbrush.png");
	texture_manager->load_texture("rectangle.png");
	texture_manager->load_texture("select.png");
	texture_manager->load_texture("stickylines.png");
	texture_manager->load_texture("save.png");
	texture_manager->load_texture("open.png");
	texture_manager->load_texture("fileicon.png", true);
	texture_manager->load_texture("conquest.png");
	texture_manager->load_texture("pressanykey.png");
	texture_manager->load_texture("ainbutton.png");
	texture_manager->load_texture("toolbarbutton.png", true);
	texture_manager->load_texture("toolbarbuttonpushed.png", true);
	texture_manager->load_texture("vacation.png", true);
	texture_manager->load_texture("vegetables.jpg", true);
	texture_manager->load_texture("sleepy.png");
	texture_manager->load_texture("itemtitle.png");
	texture_manager->load_texture("greyout.png", true);
	texture_manager->load_texture("browserdialog.png", true);
	texture_manager->load_texture("browserok.png");
	texture_manager->load_texture("browserheld.png");
	texture_manager->load_texture("browserpressed.png");
	texture_manager->load_texture("invisible.png", true);
	texture_manager->load_texture("aincircle.png", true);
	texture_manager->load_texture("surveybackground.png", true);
	texture_manager->load_texture("radiobutton.png");
	texture_manager->load_texture("radiobuttonheld.png");
	texture_manager->load_texture("radiobuttonpushed.png");
	texture_manager->load_texture("blackdot.png", true);
	texture_manager->load_texture("flowers.png", true);
	texture_manager->load_texture("fern.png", true);
	texture_manager->load_texture("coffee.png", true);
	texture_manager->load_texture("pool.png", true);
	texture_manager->load_texture("icon_mmo.png");
	texture_manager->load_texture("mmosplash.png");
	texture_manager->load_texture("recoveremail.png");
	texture_manager->load_texture("recoveremailbold.png");
	texture_manager->load_texture("recoveremailregular.png");
	texture_manager->load_texture("xbuttonlight.png");
	texture_manager->load_texture("xbuttondark.png");
	texture_manager->load_texture("minimizelight.png");
	texture_manager->load_texture("minimizedark.png");
	texture_manager->load_texture("oklight.png");
	texture_manager->load_texture("okdark.png");
	texture_manager->load_texture("golight.png");
	texture_manager->load_texture("godark.png");
	texture_manager->load_texture("genlight.png", true);
	texture_manager->load_texture("gendark.png", true);
	texture_manager->load_texture("backdark.png");
	texture_manager->load_texture("backlight.png");
	texture_manager->load_texture("forwardlight.png");
	texture_manager->load_texture("forwarddark.png");
	texture_manager->load_texture("bookmarklight.png", true);
	texture_manager->load_texture("bookmarkdark.png", true);
	texture_manager->load_texture("searchbuttonflash.png");
	texture_manager->load_texture("result.png", true);
	texture_manager->load_texture("didyoulike.png");
	texture_manager->load_texture("bigwhite.png", true);
	texture_manager->load_texture("gameover.png");
	texture_manager->load_texture("icon_console_text.png");
	texture_manager->load_texture("icon_folder_text.png");
	texture_manager->load_texture("internet_name.png");
	texture_manager->load_texture("files_name.png");
	texture_manager->load_texture("console_name.png");
	texture_manager->load_texture("helpwindow.png", true);
	texture_manager->load_texture("psychologist.png");
	texture_manager->load_texture("ellipsetip.png");
	texture_manager->load_texture("filltip.png");
	texture_manager->load_texture("filledrectangletip.png");
	texture_manager->load_texture("linetip.png");
	texture_manager->load_texture("paintbrushtip.png");
	texture_manager->load_texture("rectangletip.png");
	texture_manager->load_texture("stickylinestip.png");
	texture_manager->load_texture("savetip.png");
	texture_manager->load_texture("opentip.png");
	texture_manager->load_texture("selecttip.png");
	texture_manager->load_texture("colorpickertip.png");
	texture_manager->load_texture("icon_paint_text.png");
	texture_manager->load_texture("paint_name.png");
	texture_manager->load_texture("scrollbackground.png", true);
	texture_manager->load_texture("nendatrash.png");
	texture_manager->load_texture("nendaspam.png");
	texture_manager->load_texture("nendaabout.png");
	texture_manager->load_texture("spinnerlight.png");
	texture_manager->load_texture("spinnerdark.png");
	texture_manager->load_texture("searchspinner.png");
	texture_manager->load_texture("couldntfind.png");
	texture_manager->load_texture("holder.png");
	texture_manager->load_texture("selectcolor.png");
	texture_manager->load_texture("paintbackground.png");
	texture_manager->load_texture("buttonblank.png", true);
	texture_manager->load_texture("colorpicker.png");
	texture_manager->load_texture("spinnerorange.png", true);
	texture_manager->load_texture("energy.png", true);
	texture_manager->load_texture("spinningline.png", true);
	texture_manager->load_texture("redblock.png", true);
	texture_manager->load_texture("blueblock.png", true);
	texture_manager->load_texture("greenblock.png", true);
	texture_manager->load_texture("yellowblock.png", true);
	texture_manager->load_texture("seafoamblock.png", true);
	texture_manager->load_texture("testcomments.png", true);
	texture_manager->load_texture("bookmarkcover.png", true);
	texture_manager->load_texture("cap.png");
	texture_manager->load_texture("bookmarks.png");
	texture_manager->load_texture("bookmarkcoverdark.png");
	texture_manager->load_texture("capdark.png");
	texture_manager->load_texture("sleepynews.png");
	texture_manager->load_texture("usernamebox.png", true);
	texture_manager->load_texture("pleaseenter.png");
	texture_manager->load_texture("sleepyfooter.png");
	texture_manager->load_texture("comment1.png");
	texture_manager->load_texture("comment2.png");
	texture_manager->load_texture("comment3.png");
	texture_manager->load_texture("conversationtitle.png");
	texture_manager->load_texture("tablettitle.png");
	texture_manager->load_texture("tomthreadtitle.png");
	texture_manager->load_texture("votehasbeensubmitted.png");
	texture_manager->load_texture("useful10.png");
	texture_manager->load_texture("highscoretable.png");
	texture_manager->load_texture("helpmessage.png");
	texture_manager->load_texture("lockedfolder.png", true);
	texture_manager->load_texture("logocontesttitle.png");
	texture_manager->load_texture("contestcomment.png");
	texture_manager->load_texture("showcomments.png");
	texture_manager->load_texture("decryption.png");
	texture_manager->load_texture("splashscreen.png");
	texture_manager->load_texture("bigglass.png", false, true);
	texture_manager->load_texture("bigglassmap.png", false, true);
	texture_manager->load_texture("testpage.png", false, true);
	texture_manager->load_texture("map1.png");
	texture_manager->load_texture("tree.png", false, true);
	texture_manager->load_texture("map1overlay.png");
	texture_manager->load_texture("maindownsword.png");
	texture_manager->load_texture("mainupsword.png");
	texture_manager->load_texture("mainleftsword.png");
	texture_manager->load_texture("mainrightsword.png");
	texture_manager->load_texture("tom1.png");
	texture_manager->load_texture("tom2.png");
	texture_manager->load_texture("tom3.png");
	texture_manager->load_texture("codeicon.png", true);
	texture_manager->load_texture("blueframe1.png", true);
	texture_manager->load_texture("blueframe2.png", true);
	texture_manager->load_texture("blueframe3.png", true);
	texture_manager->load_texture("blueframe4.png", true);
	texture_manager->load_texture("blackwhitebutton.png", true);
	texture_manager->load_texture("trippy.png");
	texture_manager->load_texture("testboxes.png", false, true);
	texture_manager->load_texture("underground.png");
	texture_manager->load_texture("undergroundadd.png", false, true);
	texture_manager->load_texture("littleglass.png", false, true);
	texture_manager->load_texture("littleglassmap.png", false, true);
	texture_manager->load_texture("textfieldhover.png");
	texture_manager->load_texture("textfieldselected.png");
	texture_manager->load_texture("shadowicon.png", true);
	texture_manager->load_texture("brokentextfield.png", true);
	texture_manager->load_texture("shard1.png");
	texture_manager->load_texture("shard2.png");
	texture_manager->load_texture("shard3.png");
	texture_manager->load_texture("breakpage.png");
	texture_manager->load_texture("barbackground.png");
	texture_manager->load_texture("shinygreen.png", true);
	texture_manager->load_texture("shinyred.png", true);
	texture_manager->load_texture("emptysphere.png", true);
	texture_manager->load_texture("blueblur.png", true);
	texture_manager->load_texture("flameframe1.png", true);
	texture_manager->load_texture("flameframe2.png", true);
	texture_manager->load_texture("flameframe3.png", true);
	texture_manager->load_texture("flameframe4.png", true);
	texture_manager->load_texture("flameframe5.png", true);
	texture_manager->load_texture("alienbullet1.png", true);
	texture_manager->load_texture("alienbullet2.png", true);
	texture_manager->load_texture("tile1.png");
	texture_manager->load_texture("tile2.png");
	texture_manager->load_texture("tile3.png");
	texture_manager->load_texture("tile4.png");
	texture_manager->load_texture("tile5.png");
	texture_manager->load_texture("tile6.png");
	texture_manager->load_texture("tile7.png");
	texture_manager->load_texture("tile8.png");
	texture_manager->load_texture("goholder.png", true);
	texture_manager->load_texture("test3.png", true);
	texture_manager->load_texture("print.png", false, true);
	texture_manager->load_texture("icon_internet_tray.png");
	texture_manager->load_texture("icon_folder_tray.png");
	texture_manager->load_texture("icon_console_tray.png");
	texture_manager->load_texture("up.png");
	texture_manager->load_texture("uplight.png");
	texture_manager->load_texture("updark.png");
	texture_manager->load_texture("encryptionmessage.png");
	texture_manager->load_texture("darksilver.png");
	texture_manager->load_texture("windowcap1.png");
	texture_manager->load_texture("windowcap2.png");
	texture_manager->load_texture("qsilver.png");
	texture_manager->load_texture("qdarksilver.png");
	texture_manager->load_texture("qwindowcap1.png");
	texture_manager->load_texture("qwindowcap2.png");
	texture_manager->load_texture("smallgreycircle.png");
	texture_manager->load_texture("smallwhitecircle.png");
	texture_manager->load_texture("submitvote.png");
	texture_manager->load_texture("youwin.png");
	texture_manager->load_texture("shinygrey.png", true);
	texture_manager->load_texture("icon_paint_tray.png");
	texture_manager->load_texture("buttonblank2.png", true);
	texture_manager->load_texture("undo.png");
	texture_manager->load_texture("eraser.png");
	texture_manager->load_texture("filledellipse.png");
	texture_manager->load_texture("help.png");
	texture_manager->load_texture("checkbox.png");
	texture_manager->load_texture("checkboxlight.png");
	texture_manager->load_texture("checkboxdark.png");
	texture_manager->load_texture("check.png");
	texture_manager->load_texture("filledellipsetip.png");
	texture_manager->load_texture("erasertip.png");
	texture_manager->load_texture("undotip.png");
	texture_manager->load_texture("navholder.png");
	texture_manager->load_texture("captextfield.png");
	texture_manager->load_texture("captextfieldhover.png");
	texture_manager->load_texture("captextfieldselected.png");
	texture_manager->load_texture("spinner1.png");
	texture_manager->load_texture("spinner2.png");
	texture_manager->load_texture("altcap.png");
	texture_manager->load_texture("searchside.png");
	texture_manager->load_texture("altcaphover.png");
	texture_manager->load_texture("altcapselected.png");
	texture_manager->load_texture("altcapbutton.png");
	texture_manager->load_texture("altcapbuttonhover.png");
	texture_manager->load_texture("notsecure.png");
	texture_manager->load_texture("loadinggradient.png");
	texture_manager->load_texture("browseryes.png");
	texture_manager->load_texture("browseryesheld.png");
	texture_manager->load_texture("browseryespressed.png");
	texture_manager->load_texture("browserno.png");
	texture_manager->load_texture("browsernoheld.png");
	texture_manager->load_texture("browsernopressed.png");
	texture_manager->load_texture("didyoulike2.png");
	texture_manager->load_texture("didyoulike3.png");
	texture_manager->load_texture("connect.png");
	texture_manager->load_texture("connect2.png");
	texture_manager->load_texture("connectbar.png");
	texture_manager->load_texture("loadingbar.png");
	//texture_manager->load_texture("helpbackground.png");
	texture_manager->load_texture("helptextfield.png");
	texture_manager->load_texture("send.png");
	texture_manager->load_texture("sendheld.png");
	texture_manager->load_texture("nowconnected.png");
	texture_manager->load_texture("clouds.png");
	texture_manager->load_texture("shimmer.png");
	texture_manager->load_texture("store1.png");
	texture_manager->load_texture("storebackground.png");
	texture_manager->load_texture("bluearrow.png");
	texture_manager->load_texture("bluearrow2.png");
	texture_manager->load_texture("bluescrollbar.png");
	texture_manager->load_texture("blackline.png");
	texture_manager->load_texture("cartbutton.png");
	texture_manager->load_texture("messagesbutton.png");
	texture_manager->load_texture("ownershipbutton.png");
	texture_manager->load_texture("ownershipbuttondesat.png");
	texture_manager->load_texture("cataloguebutton.png");
	texture_manager->load_texture("lightcircle.png", true);
	texture_manager->load_texture("fireframe1.png", true);
	texture_manager->load_texture("fireframe2.png", true);
	texture_manager->load_texture("fireframe3.png", true);
	texture_manager->load_texture("fireframe4.png", true);
	texture_manager->load_texture("fireframe5.png", true);
	texture_manager->load_texture("fireframe6.png", true);
	texture_manager->load_texture("fireframe7.png", true);
	texture_manager->load_texture("fireframe8.png", true);
	texture_manager->load_texture("fireframe9.png", true);
	texture_manager->load_texture("fireframe10.png", true);
	texture_manager->load_texture("debris1.png", true);
	texture_manager->load_texture("debris2.png", true);
	texture_manager->load_texture("debris3.png", true);
	texture_manager->load_texture("debris4.png", true);
	texture_manager->load_texture("debris5.png", true);
	texture_manager->load_texture("debris6.png", true);
	texture_manager->load_texture("debris7.png", true);
	texture_manager->load_texture("debris8.png", true);
	texture_manager->load_texture("debris9.png", true);
	texture_manager->load_texture("debris10.png", true);
	texture_manager->load_texture("starwindow.png", true);
	texture_manager->load_texture("greenblur.png", true);
	texture_manager->load_texture("plaincircle.png");
	texture_manager->load_texture("bluefire.png");
	texture_manager->load_texture("circlewindow.png");
	texture_manager->load_texture("useful11.png");
	texture_manager->load_texture("useful12.png");
	texture_manager->load_texture("useful13.png");
	texture_manager->load_texture("ainlogin.png");
	texture_manager->load_texture("purchase.png");
	texture_manager->load_texture("purchasecomplete.png");
	texture_manager->load_texture("clickheretopurchase.png");
	texture_manager->load_texture("allfields.png");
	texture_manager->load_texture("advertising.png");
	texture_manager->load_texture("redalien.png");
	//for (int i = 1; i <= 20; ++i)
	//	texture_manager->load_texture("itemtitle1" + to_string(i) + ".png");
	texture_manager->load_texture("firstpage.png");
	texture_manager->load_texture("thread1.png");
	for (int i = 0; i <= 20; ++i)
		texture_manager->load_texture("titlescreen" + to_string(i) + ".png");

	for (int i = 21; i <= 40; ++i)
		texture_manager->make_alias("titlescreen" + to_string(i) + ".png", "titlescreen" + to_string(20 - (i - 20)) + ".png");

	for (int i = 0; i <= 34; ++i)
		texture_manager->load_texture("extend" + to_string(i) + ".png");

	texture_manager->load_texture("fuel.png", true);
	texture_manager->load_texture("greenbullet.png", true);
	texture_manager->load_texture("score.png");
	texture_manager->load_texture("life.png");
	texture_manager->load_texture("fueltext.png");
	texture_manager->load_texture("paul.png");
	texture_manager->load_texture("buttonsheen.png");
	texture_manager->load_texture("startgame.png");
	texture_manager->load_texture("highscores.png");
	texture_manager->load_texture("return.png");
	texture_manager->load_texture("funcorner.png");
	texture_manager->load_texture("centerracomcontact.png");
	texture_manager->load_texture("centerracompartners.png");
	texture_manager->load_texture("okblocky.png");
	texture_manager->load_texture("okblockypressed.png");
	texture_manager->load_texture("testtexture.png");
	texture_manager->load_texture("capshinyred.png");
	texture_manager->load_texture("capshinygreen.png");
	texture_manager->load_texture("capshinygrey.png");
	texture_manager->load_texture("retrygame.png");
	texture_manager->load_texture("labelbrush.png");
	texture_manager->load_texture("labelcolorpicker.png");
	texture_manager->load_texture("labelellipse.png");
	texture_manager->load_texture("labeleraser.png");
	texture_manager->load_texture("labelfill.png");
	texture_manager->load_texture("labelfilledellipse.png");
	texture_manager->load_texture("labelfilledrectangle.png");
	texture_manager->load_texture("labelline.png");
	texture_manager->load_texture("labelopen.png");
	texture_manager->load_texture("labelrectangle.png");
	texture_manager->load_texture("labelsave.png");
	texture_manager->load_texture("labelselect.png");
	texture_manager->load_texture("labelstickylines.png");
	texture_manager->load_texture("labelundo.png");
	texture_manager->load_texture("labelhelp.png");
	texture_manager->load_texture("resizer.png");
	texture_manager->load_texture("satlum.png");
	texture_manager->load_texture("circle2.png");
	texture_manager->load_texture("garble1.png");
	texture_manager->load_texture("garble2.png");
	texture_manager->load_texture("garble3.png");
	texture_manager->load_texture("header1.png");
	texture_manager->load_texture("header2.png");
	texture_manager->load_texture("header3.png");
	texture_manager->load_texture("headermaterial.png");
	texture_manager->load_texture("confirmbutton.png");
	texture_manager->load_texture("hesitatebutton.png");
	texture_manager->load_texture("buybutton.png");
	texture_manager->load_texture("buybuttondesat.png");
	texture_manager->load_texture("messagesbuttondesat.png");
	texture_manager->load_texture("messages1.png");
	texture_manager->load_texture("message1.png");
	texture_manager->load_texture("ownershipwelcome.png");
	texture_manager->load_texture("store2.png");
	texture_manager->load_texture("message2.png");
	texture_manager->load_texture("testthread.png");
	texture_manager->load_texture("forumthreads.png");
	texture_manager->load_texture("page1light.png");
	texture_manager->load_texture("page1dark.png");
	texture_manager->load_texture("page2light.png");
	texture_manager->load_texture("page2dark.png");
	texture_manager->load_texture("page198light.png");
	texture_manager->load_texture("page198dark.png");
	texture_manager->load_texture("sleepy1.png");
	texture_manager->load_texture("sleepyconversation.png");
	texture_manager->load_texture("contestannounce.png");
	texture_manager->load_texture("browsebutton.png");
	texture_manager->load_texture("browsedark.png");
	texture_manager->load_texture("uploadbutton.png");
	texture_manager->load_texture("uploaddark.png");
	texture_manager->load_texture("helptip.png");
	texture_manager->load_texture("contestheader.png");
	texture_manager->load_texture("contest1.png");
	texture_manager->load_texture("contest2.png");
	texture_manager->load_texture("contest3.png");
	texture_manager->load_texture("contest4.png");
	texture_manager->load_texture("comments0.png");
	texture_manager->load_texture("comments1.png");
	texture_manager->load_texture("comments2.png");
	texture_manager->load_texture("comments3.png");
	texture_manager->load_texture("comments4.png");
	texture_manager->load_texture("secondplace.png");
	texture_manager->load_texture("wasd.png");
	texture_manager->load_texture("sleepytablet.png");
	texture_manager->load_texture("sleepytom.png");
	texture_manager->load_texture("rfh3regular.png");
	texture_manager->load_texture("rfh3bold.png");
	texture_manager->load_texture("rfh3.png");
	texture_manager->load_texture("ain2.png");
	texture_manager->load_texture("useful20.png");
	texture_manager->load_texture("useful21.png");
	texture_manager->load_texture("useful22.png");
	texture_manager->load_texture("useful23.png");
	texture_manager->load_texture("rfh4bold.png");
	texture_manager->load_texture("rfh4regular.png");
	texture_manager->load_texture("rfh4.png");
	texture_manager->load_texture("sleepy2.png");
	texture_manager->load_texture("sitechange1.png");
	texture_manager->load_texture("sitechange2.png");
	texture_manager->load_texture("sitechange3.png");
	texture_manager->load_texture("sitechange4.png");
	texture_manager->load_texture("sitechange5.png");
	texture_manager->load_texture("sitechange6.png");
	texture_manager->load_texture("sitechange7.png");
	texture_manager->load_texture("sitechange8.png");
	texture_manager->load_texture("sitechange9.png");
	texture_manager->load_texture("sitechange10.png");
	texture_manager->load_texture("sitechange11.png");
	texture_manager->load_texture("sitechange12.png");
	texture_manager->load_texture("sitechange13.png");
	texture_manager->load_texture("sitechange14.png");
	texture_manager->load_texture("sitechange15.png");
	texture_manager->load_texture("sitechange16.png");
	texture_manager->load_texture("sitechange17.png");
	texture_manager->load_texture("sitechange18.png");
	texture_manager->load_texture("useful30.png");
	texture_manager->load_texture("useful31.png");
	texture_manager->load_texture("useful32.png");
	texture_manager->load_texture("useful33.png");
	texture_manager->load_texture("ain3.png");
	texture_manager->load_texture("thread11.png");
	texture_manager->load_texture("thread12.png");
	texture_manager->load_texture("thread21.png");
	texture_manager->load_texture("thread22.png");
	texture_manager->load_texture("thread31.png");
	texture_manager->load_texture("thread32.png");
	texture_manager->load_texture("thread41.png");
	texture_manager->load_texture("thread51.png");
	texture_manager->load_texture("thread52.png");
	texture_manager->load_texture("surveycolor.png");
	texture_manager->load_texture("sleepy3.png");
	texture_manager->load_texture("hcthread.png");
	texture_manager->load_texture("rfh5.png");
	texture_manager->load_texture("rfh5bold.png");
	texture_manager->load_texture("rfh5regular.png");
	texture_manager->load_texture("imagehost.png");
	texture_manager->load_texture("useful40.png");
	texture_manager->load_texture("useful41.png");
	texture_manager->load_texture("useful42.png");
	texture_manager->load_texture("useful43.png");
	texture_manager->load_texture("rfh6.png");
	texture_manager->load_texture("rfh6bold.png");
	texture_manager->load_texture("rfh6regular.png");
	texture_manager->load_texture("rosecolored6.png");
	texture_manager->load_texture("rosecolored6bold.png");
	texture_manager->load_texture("rosecolored6regular.png");
	texture_manager->load_texture("certify.png");
	texture_manager->load_texture("sleepyred.png");
	texture_manager->load_texture("adminnotice.png");
	texture_manager->load_texture("linesexist.png");
	texture_manager->load_texture("squaresexist.png");
	texture_manager->load_texture("brandonexist.png");
	texture_manager->load_texture("existconquest.png");
	texture_manager->load_texture("deliveryexist.png");
	texture_manager->load_texture("typingexist.png");
	texture_manager->load_texture("icon_decrypt_text.png");
	texture_manager->load_texture("icon_decrypt_tray.png");
	texture_manager->load_texture("decryptbackground.png");
	texture_manager->load_texture("qdecrypt1.png");
	texture_manager->load_texture("eula.png");
	texture_manager->load_texture("okblockygrey.png");
	texture_manager->load_texture("qdecrypt2.png");
	texture_manager->load_texture("qdecrypt3.png");
	texture_manager->load_texture("textfieldblocky.png");
	texture_manager->load_texture("qdecrypt4.png");
	texture_manager->load_texture("qdecrypt5.png");
	texture_manager->load_texture("qdecrypt6.png");
	texture_manager->load_texture("qdecrypt7.png");
	texture_manager->load_texture("qdecrypt8.png");
	texture_manager->load_texture("qdecrypt9.png");
	texture_manager->load_texture("cursor.png");
	texture_manager->load_texture("cursori.png");
	texture_manager->load_texture("cursorhand.png");
	texture_manager->load_texture("overlay.png");
	texture_manager->load_texture("red2.png");
	texture_manager->load_texture("disputebg.png");
	texture_manager->load_texture("disputetext.png");
	texture_manager->load_texture("dispute1.png");
	texture_manager->load_texture("dispute2.png");
	texture_manager->load_texture("os1n.png");
	texture_manager->load_texture("os2n.png");
	texture_manager->load_texture("os3n.png");
	texture_manager->load_texture("os4n.png");
	texture_manager->load_texture("os1h.png");
	texture_manager->load_texture("os2h.png");
	texture_manager->load_texture("os3h.png");
	texture_manager->load_texture("os4h.png");
	texture_manager->load_texture("disputethinking.png");
	texture_manager->load_texture("redx.png");
	texture_manager->load_texture("greencheck.png");
	texture_manager->load_texture("nextbutton.png");
	texture_manager->load_texture("testessay.png");
	texture_manager->load_texture("testred.png");
	texture_manager->load_texture("testblue.png");
	texture_manager->load_texture("bluecap1.png");
	texture_manager->load_texture("bluecap2.png");
	texture_manager->load_texture("scrollbackground2.png");
	texture_manager->load_texture("flowers.png");
	texture_manager->load_texture("chair.png");
	texture_manager->load_texture("extinguisher.png");
	texture_manager->load_texture("itembackground.png");
	texture_manager->load_texture("metalholder.png");
	texture_manager->load_texture("fadeout1.png");
	texture_manager->load_texture("gameborder.png");
	for (int i = 1; i <= 13; ++i)
		texture_manager->load_texture("scores" + to_string(i) + ".png");

	for (int i = 1; i <= 120; ++i)
	{
		string base = to_string(i);
		string zeros = string(4 - base.size(), '0');
		texture_manager->load_texture(zeros + base + ".png");
	}

	texture_manager->load_texture("brushcursor.png");
	texture_manager->load_texture("cursorfill.png");
	texture_manager->load_texture("cursorpicker.png");
	texture_manager->load_texture("cursormove.png");
	texture_manager->load_texture("useful.png");
	texture_manager->load_texture("usefulmessage1.png");
	texture_manager->load_texture("cursoriwhite.png");
	texture_manager->load_texture("usefulmessage2.png");
	texture_manager->load_texture("usefulmessage3.png");
	texture_manager->load_texture("trialexpired.png");
	texture_manager->load_texture("usefulmessage4.png");
	texture_manager->load_texture("arrowlight.png");
	texture_manager->load_texture("arrowdark.png");
	texture_manager->load_texture("arrow2light.png");
	texture_manager->load_texture("arrow2dark.png");
	texture_manager->load_texture("systemtextfield.png");
	texture_manager->load_texture("home.png");
	texture_manager->load_texture("homelight.png");
	texture_manager->load_texture("homedark.png");
	texture_manager->load_texture("bookmarksbutton.png");
	texture_manager->load_texture("bookmarksbuttondark.png");
	texture_manager->load_texture("bookmarksbuttonlight.png");
	texture_manager->load_texture("bookmarkholder.png");
	texture_manager->load_texture("searchbookmarkdark.png");
	texture_manager->load_texture("ainbuttondark.png");
	texture_manager->load_texture("socnewsbookmarkdark.png");
	texture_manager->load_texture("nendabookmarkdark.png");
	texture_manager->load_texture("usefulbookmarkdark.png");
	texture_manager->load_texture("ainbookmark.png");
	texture_manager->load_texture("ainbookmarkdark.png");
	texture_manager->load_texture("windowborder1.png");
	texture_manager->load_texture("windowborder2.png");
	texture_manager->load_texture("corner1.png");
	texture_manager->load_texture("corner2.png");
	texture_manager->load_texture("corner3.png");
	texture_manager->load_texture("corner4.png");
	for (int i = 1; i <= 59; ++i)
		texture_manager->load_texture("lock" + to_string(i) + ".png");

	texture_manager->load_texture("infobg.png");
	texture_manager->load_texture("encryptionlabel.png");
	texture_manager->load_texture("ceiling.png");
	texture_manager->load_texture("tiles.png");
	texture_manager->load_texture("bluebackground.png");
	texture_manager->load_texture("title.png");
	texture_manager->load_texture("newgame1.png");
	texture_manager->load_texture("newgame2.png");
	texture_manager->load_texture("continue1.png");
	texture_manager->load_texture("continue2.png");
	texture_manager->load_texture("filenormal.png");
	texture_manager->load_texture("filelight.png");
	texture_manager->load_texture("filedark.png");
	texture_manager->load_texture("return1.png");
	texture_manager->load_texture("return2.png");
	texture_manager->load_texture("delete1.png");
	texture_manager->load_texture("delete2.png");
	texture_manager->load_texture("deletemenu.png");
	texture_manager->load_texture("yes1.png");
	texture_manager->load_texture("yes2.png");
	texture_manager->load_texture("no1.png");
	texture_manager->load_texture("no2.png");
	texture_manager->load_texture("cursorleftright.png");
	texture_manager->load_texture("carheader.png");
	texture_manager->load_texture("emailaddress.png");
	texture_manager->load_texture("validdate.png");
	texture_manager->load_texture("validcard.png");
	texture_manager->load_texture("errorface.png");
	texture_manager->load_texture("sphere.png");
	texture_manager->load_texture("upload.png");
	texture_manager->load_texture("defaultresults.png");
	texture_manager->load_texture("walter1.png");
	texture_manager->load_texture("404message.png");
	texture_manager->load_texture("404.png");
	texture_manager->load_texture("walter2.png");
	texture_manager->load_texture("showcomments1.png");
	texture_manager->load_texture("showcomments2.png");
	texture_manager->load_texture("showcomments3.png");
	texture_manager->load_texture("walterface.png");
	texture_manager->load_texture("doyouwanttoquit.png");
	texture_manager->load_texture("centerracominvalid.png");
	texture_manager->load_texture("walter3.png");
	texture_manager->load_texture("decryptthread.png");
	texture_manager->load_texture("rfh7.png");
	texture_manager->load_texture("rfh7bold.png");
	texture_manager->load_texture("rfh7regular.png");
	texture_manager->load_texture("usefulmessage5.png");
	texture_manager->load_texture("title2.png");
}

void init_light()
{
	GLfloat light_position[] = {current_light_x, current_light_y, current_light_z, 1.0};
	GLfloat light_intensity[] = {current_intensity, current_intensity, current_intensity, 1.0}; //4.5
	GLfloat light_pos[] = { 330, 100, 0, 1.0 };
	GLfloat light_ins[] = { 1.0, 1.0, 1.0, 1.0 };
	if (blue_light)
	{
		light_intensity[0] = 0.0;
		light_intensity[1] = 0.0;
	}

	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_intensity);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_intensity);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 5.0); //8?
	glLightfv(GL_LIGHT1, GL_POSITION, light_pos);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_ins);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_ins);
	glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION, 5.0); //8?
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
}

void reshape_callback(int width, int height)
{
	glViewport(0, 0, width, height);
	camera->window_resized(width, height);
	computer->window_resized(width, height);
	pause_menu->window_resized(width, height);
	glutPostRedisplay();
}

void draw_wall()
{
	float emission[4] = {0.0, 0.0, 0.0, 1.0};
	//glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	float horiz_step = (WALL_DISTANCE * 2) / WALL_QUADS_HORIZ;
	float vert_step = WALL_HEIGHT / WALL_QUADS_VERT;
	glPushMatrix();
	glTranslatef(-WALL_DISTANCE, 0.0, 0.0);
	/*for (int i = 0; i < WALL_QUADS_HORIZ; ++i)
	{
		for (int j = 0; j < WALL_QUADS_VERT; ++j)
		{
			float horiz_percent = (float) i / (float) WALL_QUADS_HORIZ;
			float vert_percent = (float) j / (float) WALL_QUADS_VERT;
			float next_horiz_percent = (float) (i + 1) / (float) WALL_QUADS_HORIZ;
			float next_vert_percent = (float) (j + 1) / (float) WALL_QUADS_VERT;
			glBegin(GL_QUADS);
				glTexCoord2f(WALL_TEXTURE_S * horiz_percent, WALL_TEXTURE_T * vert_percent); glNormal3f(0.0, 0.0, 1.0); glVertex3f(0.0, 0.0, 0.0);
				glTexCoord2f(WALL_TEXTURE_S * horiz_percent, WALL_TEXTURE_T * next_vert_percent); glNormal3f(0.0, 0.0, 1.0); glVertex3f(0.0, vert_step, 0.0);
				glTexCoord2f(WALL_TEXTURE_S * next_horiz_percent, WALL_TEXTURE_T * next_vert_percent); glNormal3f(0.0, 0.0, 1.0); glVertex3f(horiz_step, vert_step, 0.0);
				glTexCoord2f(WALL_TEXTURE_S * next_horiz_percent, WALL_TEXTURE_T * vert_percent); glNormal3f(0.0, 0.0, 1.0); glVertex3f(horiz_step, 0.0, 0.0);
			glEnd();
			glTranslatef(0.0, vert_step, 0.0);
		}

		glTranslatef(0.0, -WALL_HEIGHT, 0.0);
		glTranslatef(horiz_step, 0.0, 0.0);
	}*/

	glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glNormal3f(0.0, 0.0, 1.0); glVertex3f(0.0, 0.0, 0.0);
		glTexCoord2f(0, 1); glNormal3f(0.0, 0.0, 1.0); glVertex3f(0.0, WALL_HEIGHT, 0.0);
		glTexCoord2f(1, 1); glNormal3f(0.0, 0.0, 1.0); glVertex3f(WALL_DISTANCE * 2, WALL_HEIGHT, 0.0);
		glTexCoord2f(1, 0); glNormal3f(0.0, 0.0, 1.0); glVertex3f(WALL_DISTANCE * 2, 0.0, 0.0);
	glEnd();
	glPopMatrix();
}

void draw_floor(bool ceiling)
{
	float emission[4] = {0.4, 0.4, 0.4, 1.0};
	float default_diffuse[4] = {0.8, 0.8, 0.8, 1.0};
	//glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
	/*if (!ceiling)
	{
		float diffuse[4] = {0.4, 0.4, 0.4, 1.0};
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
	}*/

	//GLfloat light_intensity[] = { 5.0, 5.0, 5.0, 1.0 };
	//GLfloat light_position[] = { 0.0, WALL_HEIGHT * 2, 0.0, 1.0 };
	//glLightfv(GL_LIGHT0, GL_DIFFUSE, light_intensity);
	//glLightfv(GL_LIGHT0, GL_SPECULAR, light_intensity);
	//glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	float horiz_step = (WALL_DISTANCE * 2) / FLOOR_QUADS_HORIZ;
	float vert_step = (WALL_DISTANCE * 2) / FLOOR_QUADS_VERT;
	glPushMatrix();
	glTranslatef(-WALL_DISTANCE, 0.0, WALL_DISTANCE);
	/*for (int i = 0; i < FLOOR_QUADS_HORIZ; ++i)
	{
		for (int j = 0; j < FLOOR_QUADS_VERT; ++j)
		{
			float horiz_percent = (float) i / (float) FLOOR_QUADS_HORIZ;
			float vert_percent = (float) j / (float) FLOOR_QUADS_VERT;
			float next_horiz_percent = (float) (i + 1) / (float) FLOOR_QUADS_HORIZ;
			float next_vert_percent = (float) (j + 1) / (float) FLOOR_QUADS_VERT;
			glBegin(GL_QUADS);
				glTexCoord2f(FLOOR_TEXTURE_S * horiz_percent, FLOOR_TEXTURE_T * vert_percent); glNormal3f(0.0, 1.0, 0.0); glVertex3f(0.0, 0.0, 0.0);
				glTexCoord2f(FLOOR_TEXTURE_S * horiz_percent, FLOOR_TEXTURE_T * next_vert_percent); glNormal3f(0.0, 1.0, 0.0); glVertex3f(0.0, 0.0, -vert_step);
				glTexCoord2f(FLOOR_TEXTURE_S * next_horiz_percent, FLOOR_TEXTURE_T * next_vert_percent); glNormal3f(0.0, 1.0, 0.0); glVertex3f(horiz_step, 0.0, -vert_step);
				glTexCoord2f(FLOOR_TEXTURE_S * next_horiz_percent, FLOOR_TEXTURE_T * vert_percent); glNormal3f(0.0,  1.0, 0.0); glVertex3f(horiz_step, 0.0, 0.0);
			glEnd();
			glTranslatef(0.0, 0.0, -vert_step);
		}

		glTranslatef(0.0, 0.0, WALL_DISTANCE * 2);
		glTranslatef(horiz_step, 0.0, 0.0);
	}*/

	glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glNormal3f(0.0, 1.0, 0.0); glVertex3f(0.0, 0.0, 0.0);
		glTexCoord2f(0, 1); glNormal3f(0.0, 1.0, 0.0); glVertex3f(0.0, 0.0, -WALL_DISTANCE * 2);
		glTexCoord2f(1, 1); glNormal3f(0.0, 1.0, 0.0); glVertex3f(WALL_DISTANCE * 2, 0.0, -WALL_DISTANCE * 2);
		glTexCoord2f(1, 0); glNormal3f(0.0, 1.0, 0.0); glVertex3f(WALL_DISTANCE * 2, 0.0, 0.0);
	glEnd();
	glPopMatrix();
	//glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, default_diffuse);
}

void draw_bulb()
{
	/*glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	texture_manager->change_texture("light.png");
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(-10.0, 190.0, -4.0);
		glTexCoord2f(0.0, 1.0); glVertex3f(-10.0, 200.0, -4.0);
		glTexCoord2f(1.0, 1.0); glVertex3f(10.0, 200.0, -4.0);
		glTexCoord2f(1.0, 0.0); glVertex3f(10.0, 190.0, -4.0);
	glEnd();*/
	//glDisable(GL_LIGHTING);
	//float ambient[4] = { 3.0, 3.0, 3.0, 1.0 };
	//float default_ambient[4] = { 0.2, 0.2, 0.2, 1.0 };
	//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, ambient);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glPushMatrix();
		glTranslatef(0.0, WALL_HEIGHT - 9.0, 0.0); //WALL_HEIGHT - 20
		glRotatef(180.0, 0.0, 0.0, 1.0);
		glScalef(1.5, 1.5, 1.5);
		bulb->draw();
	glPopMatrix();
	//glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, default_ambient);
	glEnable(GL_LIGHTING);
}

void draw_beads()
{
	float default_emission[4] = { 0.0, 0.0, 0.0, 1.0 };
	float bead_emission[4] = { 0.3, 0.3, 0.3, 1.0 };
	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, bead_emission);
		glTranslatef(0.0, 268.3, -4.2);
		for (int i = 0; i < 10; ++i)
		{
			glutSolidSphere(0.65, 20, 20);
			glTranslatef(0.0, 1.4, 0.0);
		}

		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, default_emission);
	glPopMatrix();
	glEnable(GL_TEXTURE_2D);
}

void draw_room()
{
	GLfloat light_position[] = { 0.0, static_cast<GLfloat>(WALL_HEIGHT - 81.0), 0.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	texture_manager->change_texture("tiles.png");
	if (game_state != STATE_FLYING)
	{
		glPushMatrix();
		glTranslatef(0.0, 0.0, -WALL_DISTANCE);
		draw_wall();
		glPopMatrix();

		glPushMatrix();
		glTranslatef(-WALL_DISTANCE, 0.0, 0.0);
		glRotatef(90.0, 0.0, 1.0, 0.0);
		draw_wall();
		glPopMatrix();
	}

	glPushMatrix();
		glTranslatef(WALL_DISTANCE, 0.0, 0.0);
		glRotatef(-90.0, 0.0, 1.0, 0.0);
		draw_wall();
	glPopMatrix();

	glPushMatrix();
		glTranslatef(0.0, 0.0, WALL_DISTANCE);
		glRotatef(180.0, 0.0, 1.0, 0.0);
		draw_wall();
	glPopMatrix();

	light_position[1] = WALL_HEIGHT * 2.0;
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	
	texture_manager->change_texture("ceiling.png");
	draw_floor(false);
	//draw_beads();

	light_position[1] = WALL_HEIGHT * -2.0;
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	if (game_state != STATE_FLYING)
	{
		glPushMatrix();
		glTranslatef(0.0, WALL_HEIGHT, 0.0);
		glRotatef(180.0, 0.0, 0.0, 1.0);
		texture_manager->change_texture("ceiling.png");
		draw_floor(true);
		glPopMatrix();
	}

	draw_bulb();

	//TODO move shadow when computer moves
	if (game_state != STATE_NO_COMPUTER && draw_computer)
	{
		glPopMatrix();
		glPushMatrix(); //NEW
		glTranslatef(380.0, 0.0, 0.0);
		glDisable(GL_LIGHTING);
		texture_manager->change_texture("shadow.png");
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex3f(-90.0, 0.5, -52.0);
		glTexCoord2f(0.0, 1.0); glVertex3f(-90.0, 0.5, 52.0);
		glTexCoord2f(1.0, 1.0); glVertex3f(30.0, 0.5, 52.0);
		glTexCoord2f(1.0, 0.0); glVertex3f(30.0, 0.5, -52.0);
		glEnd();
		glEnable(GL_LIGHTING);
		glPopMatrix(); //NEW
	}
}

void draw_desk()
{
	if (!draw_computer)
		return;
	
	float emission[4] = { 0.4, 0.4, 0.4, 1.0 }; //0.4
	float default_emission[4] = { 0.0, 0.0, 0.0, 1.0 };
	glPushMatrix();
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
		glTranslatef(computer_setups[computer_setup_index].x, 32.0, computer_setups[computer_setup_index].z);
		glScalef(20.0, 20.0, 20.0);
		//desk->draw();
		glTranslatef(0.0, 1.0, 0.0);
		glRotatef(computer_setups[computer_setup_index].y, 0.0, 1.0, 0.0); //if you take this out so the display side gets less light, that side looks nice
		monitor->draw();
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, default_emission);
	glPopMatrix();
}

void draw_wasd()
{
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);
	glOrtho(0.0, width, height, 0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	texture_manager->change_texture("wasd.png");
	glColor4f(1.0, 1.0, 1.0, wasd_alpha);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(scalex(200), scaley(200));
	glTexCoord2f(0.0, 0.99); glVertex2f(scalex(200), scaley(400));
	glTexCoord2f(1.0, 0.99); glVertex2f(scalex(500), scaley(400));
	glTexCoord2f(1.0, 0.0); glVertex2f(scalex(500), scaley(200));
	glEnd();
	glPopMatrix();
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glColor4f(1.0, 1.0, 1.0, 1.0);
}

void draw_eyelids()
{
	glDisable(GL_LIGHTING);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);
	glOrtho(0.0, width, height, 0.0, -1.0, 1.0);
	//glOrtho(0.0, width, 0.0, height, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	//glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//ScreenElement white(0, 0, width, height, "white.png");
	//white.draw(texture_manager.get());
	if (eyelid_offset <= 120)
	{
		glColor4f(1.0, 1.0, 1.0, eyelid_offset <= 110 ? 1.0 : (120 - eyelid_offset) * 10.0 / 100.0);
		ScreenElement black(0, 0, 1920, 1080, "black.png");
		black.draw(texture_manager.get());
		glColor4f(1.0, 1.0, 1.0, 1.0);
	}

	texture_manager->change_texture("test.png");
	glPushMatrix();
	glTranslatef(0.0, -eyelid_offset, 0.0);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0); glVertex2f(0.0, 0.0);
	glTexCoord2f(0.0, 0.99); glVertex2f(0.0, height);
	glTexCoord2f(1.0, 0.99); glVertex2f(width, height);
	glTexCoord2f(1.0, 0.0); glVertex2f(width, 0.0);
	glEnd();
	glPopMatrix();
	glPushMatrix();
	glTranslatef(0.0, eyelid_offset, 0.0);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.99); glVertex2f(0.0, 0.0);
	glTexCoord2f(0.0, 0.0); glVertex2f(0.0, height);
	glTexCoord2f(1.0, 0.0); glVertex2f(width, height);
	glTexCoord2f(1.0, 0.99); glVertex2f(width, 0.0);
	glEnd();

	
	/*else
	{
		ScreenElement white(0, 0, 1920, 1080, "white.png");
		//white.draw(texture_manager.get());
		glTranslatef(0.0, 540.0, 0.0);
		int sign = 1;
		for (int k = 0; k < 2; k++)
		{
			ScreenElement rect(0, sign * eyelid_offset, 1920, sign * eyelid_offset + sign * 540, "black.png");
			rect.draw(texture_manager.get());
			int cur_y = sign * eyelid_offset;
			float alpha = 1.0;
			int line_count = 0;
			float alpha_inc = -0.002;
			for (;;) //0.002
			{
				ScreenElement line(0, cur_y, 1920, cur_y + sign, "black.png");
				glColor4f(1.0, 1.0, 1.0, alpha);
				line.draw(texture_manager.get());
				glColor4f(1.0, 1.0, 1.0, 1.0);
				cur_y -= sign;
				line_count++;
				if (abs(cur_y) > 40)
					alpha += alpha_inc;

				if (abs(cur_y) < 40 && cur_y % 3 == 0)
					alpha_inc *= -1.0;

				if (alpha <= 0.0)
				{

					break;
				}

				if ((cur_y == -1 && sign == 1) || (cur_y == 1 && sign == -1))
				{
					//line.name = "red.png";
					//glColor4f(1.0, 1.0, 1.0, 1.0);
					//line.draw(texture_manager.get());
					break;
				}
			}

			sign = -1;
		}
	}*/

	glPopMatrix();
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void display_callback()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //calling blend in eyelids is redundant
	if (game_state == STATE_NO_COMPUTER || game_state == STATE_WALKING || game_state == STATE_AWAKENING || game_state == STATE_GETTING_UP || game_state == STATE_FLYING)
	{
		glutSetCursor(GLUT_CURSOR_NONE);
		camera->setup_view();
		init_light();
		if (game_state == STATE_WALKING)
			draw_desk();

		if (game_state != STATE_FLYING)
			draw_room();

		if (game_state == STATE_NO_COMPUTER || game_state == STATE_WALKING)
			draw_wasd();
	}

	if (game_state == STATE_FLYING)
	{
		for (int i = 0; i < camera->draw_planes.size(); ++i)
		{
			texture_manager->change_texture(camera->draw_planes[i].name);
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0); glVertex3f(camera->draw_planes[i].p1.x, camera->draw_planes[i].p1.y, camera->draw_planes[i].p1.z);
			glTexCoord2f(1, 0); glVertex3f(camera->draw_planes[i].p2.x, camera->draw_planes[i].p2.y, camera->draw_planes[i].p2.z);
			glTexCoord2f(1, 1); glVertex3f(camera->draw_planes[i].p4.x, camera->draw_planes[i].p4.y, camera->draw_planes[i].p4.z);
			glTexCoord2f(0, 1); glVertex3f(camera->draw_planes[i].p3.x, camera->draw_planes[i].p3.y, camera->draw_planes[i].p3.z);
			glEnd();
		}

		/*texture_manager->change_texture("blue.png");
		for (int i = 0; i < camera->planes.size(); ++i)
		{
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0); glVertex3f(camera->planes[i].p1.x, camera->planes[i].p1.y, camera->planes[i].p1.z);
			glTexCoord2f(1, 0); glVertex3f(camera->planes[i].p2.x, camera->planes[i].p2.y, camera->planes[i].p2.z);
			glTexCoord2f(1, 1); glVertex3f(camera->planes[i].p4.x, camera->planes[i].p4.y, camera->planes[i].p4.z);
			glTexCoord2f(0, 1); glVertex3f(camera->planes[i].p3.x, camera->planes[i].p3.y, camera->planes[i].p3.z);
			glEnd();
		}*/
	}

	if (game_state == STATE_COMPUTER)
	{
		if (new_frames == 0)
		{
			start = time(NULL);
			end_t = time(NULL);
		}
		
		computer->setup_view();
		computer->draw();
	}

	if (game_state == STATE_AWAKENING || game_state == STATE_GETTING_UP)
	{
		draw_eyelids();
	}

	if (game_state == STATE_FLYING)
	{
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		if (camera->rotating_quad)
		{
			texture_manager->change_texture("underground.png");
			float width = texture_manager->get_width("underground.png");
			float height = texture_manager->get_height("underground.png");
			float lowx = abs(computer->mmo_stage_x1) / width;
			float highx = (abs(computer->mmo_stage_x1) + 1920) / width;
			float lowy = abs(computer->mmo_stage_y1) / height;
			float highy = (abs(computer->mmo_stage_y1) + 1080) / height;
			float ydist = tan((50.0 / 2.0) * PI / 180) * 2;
			float zdist = ydist * camera->aspect;
			float fhighy = 1.0 - highy;
			float flowy = 1.0 - lowy;
			glPushMatrix();
			glTranslatef(-898, -ydist, 0.0);
			glRotatef(camera->rotation_angle, 0.0, 0.0, -1.0);
			glBegin(GL_QUADS);
			glTexCoord2f(lowx, flowy); glVertex3f(0, 0, -zdist);
			glTexCoord2f(highx, flowy); glVertex3f(0, 0, zdist);
			glTexCoord2f(highx, fhighy); glVertex3f(0, ydist*2, zdist);
			glTexCoord2f(lowx, fhighy); glVertex3f(0, ydist*2, -zdist);
			glEnd();
			float nx1 = (computer->mmo_player_x1 / 1920.0) * zdist * 2.0 - zdist;
			float nx2 = ((computer->mmo_player_x1 + 14) / 1920.0) * zdist * 2.0 - zdist;
			float ny1 = (computer->mmo_player_y1 / 1080.0) * ydist * 2.0;
			float ny2 = ((computer->mmo_player_y1 + 22) / 1080.0) * ydist * 2.0;
			texture_manager->change_texture("cursor.png");
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0); glVertex3f(0, ny2, nx1);
			glTexCoord2f(0, 1); glVertex3f(0, ny1, nx1);
			glTexCoord2f(1, 1); glVertex3f(0, ny1, nx2);
			glTexCoord2f(1, 0); glVertex3f(0, ny2, nx2);
			glEnd();
			glPopMatrix();
		}

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		int width = glutGet(GLUT_WINDOW_WIDTH);
		int height = glutGet(GLUT_WINDOW_HEIGHT);
		glOrtho(0.0, width, 0.0, height, -1.0, 1.0);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		texture_manager->change_texture("overlay.png");
		glBegin(GL_QUADS);
			glTexCoord2f(0.0, 1.0); glVertex2f(0.0, 0);
			glTexCoord2f(0.0, 0); glVertex2f(0.0, 1080);
			glTexCoord2f(1.0, 0); glVertex2f(1920, 1080);
			glTexCoord2f(1.0, 1.0); glVertex2f(1920, 0);
		glEnd();
		glEnable(GL_DEPTH_TEST);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}

	if (game_state == STATE_LOADING)
	{
		glutSetCursor(GLUT_CURSOR_NONE);
		glDisable(GL_LIGHTING);
		glDisable(GL_DEPTH_TEST);
		computer->setup_view();
		texture_manager->change_texture("nowloading.png");
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 1.0); glVertex2f(scalex(1500), scaley(300));
		glTexCoord2f(0.0, 0); glVertex2f(scalex(1500), scaley(337));
		glTexCoord2f(1.0, 0); glVertex2f(scalex(1500 + 259), scaley(337));
		glTexCoord2f(1.0, 1.0); glVertex2f(scalex(1500 + 259), scaley(300));
		glEnd();
		texture_manager->change_texture("titleloading.png");
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 1.0); glVertex2f(scalex(1500), scaley(200));
		glTexCoord2f(0.0, 0); glVertex2f(scalex(1500), scaley(237));
		glTexCoord2f(1.0, 0); glVertex2f(scalex(1500 + 259), scaley(237));
		glTexCoord2f(1.0, 1.0); glVertex2f(scalex(1500 + 259), scaley(200));
		glEnd();
		texture_manager->change_texture("titleloadingfill.png");
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 1.0); glVertex2f(scalex(1500), scaley(200));
		glTexCoord2f(0.0, 0); glVertex2f(scalex(1500), scaley(237));
		glTexCoord2f(loading_percentage, 0); glVertex2f(scalex(1500 + loading_percentage * 259), scaley(237));
		glTexCoord2f(loading_percentage, 1.0); glVertex2f(scalex(1500 + loading_percentage * 259), scaley(200));
		glEnd();
	}

	if (do_pause_menu)
	{
		pause_menu->setup_view();
		pause_menu->draw();
	}

	glDisable(GL_TEXTURE_2D);
	glutSwapBuffers();
	//glDepthMask(GL_TRUE);
	//glEnable(GL_DEPTH_TEST);
}

void keyboard_callback(unsigned char key, int x, int y)
{
	if (key == 27)
	{
		do_pause_menu = true;
		pause_menu->pause_done = false;
		for (int i = 0; i <= 255; ++i)
		{
			computer->release_key(i);
			camera->release_key(i);
		}

		computer->mouse_clicked(GLUT_LEFT, GLUT_UP, computer->prev_mouse_x, 1080 - computer->prev_mouse_y);
		computer->mouse_clicked(GLUT_RIGHT_BUTTON, GLUT_UP, computer->prev_mouse_x, 1080 - computer->prev_mouse_y);
		glutWarpPointer(pause_menu->prev_mouse_x, 1080 - pause_menu->prev_mouse_y);
	}

	/*if (key == 'p')
		high_y += 0.1;

	if (key == 'o')
		high_y -= 0.1;

	if (key == 'l')
		low_y += 0.1;

	if (key == 'k')
		low_y -= 0.1;*/

	//if (key == 'z')
	//		save_game(computer->save_slot);
		//computer->update_in_progress = !(computer->update_in_progress);

	//if (key == 'x')
	//	load_game(computer->save_slot);

	//if (key == 'y')
	//	camera->start_file_animation("anim1");

	//if (key == 'k')
	//	computer->stop_sound(6);

	if ((key == 'n' && game_state == STATE_AWAKENING || game_state == STATE_GETTING_UP) && true && DEBUG)
	{
		game_state = STATE_WALKING;
		camera->camera_y = INITIAL_WALKING_Y;
		camera->look_x = 0.0;
		camera->look_y = 1.0;
		camera->look_z = 0.0;
		camera->computer_present = true;
		camera->computer_planes();
	}

	/*if (key == 'l')
	{
		lights_flickering = true;
	}

	if (key == 'p')
	{
		if (game_state != STATE_COMPUTER)
		{
			computer->reset_computer();
			glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
			game_state = STATE_COMPUTER;
		}

		else
			game_state = STATE_WALKING;
	}*/

	if ((game_state == STATE_NO_COMPUTER || game_state == STATE_WALKING || game_state == STATE_FLYING) && !do_pause_menu)
	{
		camera->press_key(key);
		if (wasd_alpha_inc > 0)
		{
			wasd_alpha_inc *= -1;
			wasd_alpha += wasd_alpha_inc * 2;
			if (wasd_alpha < 0)
				wasd_alpha = 0;
		}
	}

	else if (game_state == STATE_COMPUTER && !do_pause_menu)
		computer->press_key(key);
}

void special_keyboard_callback(int key, int x, int y)
{
	unsigned char new_key;
	switch (key)
	{
	case GLUT_KEY_LEFT:
		new_key = KEY_LEFT;
		break;

	case GLUT_KEY_UP:
		new_key = KEY_UP;
		break;

	case GLUT_KEY_RIGHT:
		new_key = KEY_RIGHT;
		break;

	case GLUT_KEY_DOWN:
		new_key = KEY_DOWN;
		break;

	default:
		return;
	}

	keyboard_callback(new_key, x, y);
}

void keyboard_up_callback(unsigned char key, int x, int y)
{
	if (game_state == STATE_NO_COMPUTER || game_state == STATE_WALKING || game_state == STATE_FLYING)
		camera->release_key(key);

	else if (game_state == STATE_COMPUTER)
		computer->release_key(key);
}

void special_keyboard_up_callback(int key, int x, int y)
{
	unsigned char new_key;
	switch (key)
	{
	case GLUT_KEY_LEFT:
		new_key = KEY_LEFT;
		break;

	case GLUT_KEY_UP:
		new_key = KEY_UP;
		break;

	case GLUT_KEY_RIGHT:
		new_key = KEY_RIGHT;
		break;

	case GLUT_KEY_DOWN:
		new_key = KEY_DOWN;
		break;

	default:
		return;
	}

	keyboard_up_callback(new_key, x, y);
}

void passive_motion_callback(int x, int y)
{
	if ((game_state == STATE_NO_COMPUTER || game_state == STATE_WALKING || game_state == STATE_FLYING) && !do_pause_menu)
	{
		camera->mouse_moved(x, y);
	}

	if (game_state == STATE_COMPUTER && !do_pause_menu)
	{
		computer->mouse_moved(x, y);
	}

	if (do_pause_menu)
	{
		pause_menu->mouse_moved(x, y);
	}
}

void mouse_callback(int button, int state, int x, int y)
{
	//cout << to_string(x) + ", " + to_string(glutGet(GLUT_WINDOW_HEIGHT) - y) + "\n";
	if (game_state == STATE_COMPUTER && !do_pause_menu)
		computer->mouse_clicked(button, state, x, y);

	if (do_pause_menu)
		pause_menu->mouse_clicked(button, state, x, y);
}

float dist(float x1, float y1, float x2, float y2)
{
	return sqrt(pow(y2 - y1, 2) + pow(x2 - x1, 2));
}

void timer_callback(int value)
{
	if (computer->email_counter > 0)
		computer->email_counter++;
	
	if (game_state != STATE_LOADING && !(game_state == STATE_COMPUTER && computer->computer_state == STATE_TITLE_SCREEN))
	{
		int new_tick = glutGet(GLUT_ELAPSED_TIME);
		computer->elapsed_time += new_tick - computer->last_tick;
		computer->last_tick = new_tick;
	}
	
	if (game_state == STATE_FLYING && camera->return_from_flying)
	{
		game_state = STATE_COMPUTER;
		camera->return_from_flying = false;
		bool s = false;
		for (int i = 0; i < camera->keys_pressed.size(); ++i)
		{
			if (camera->keys_pressed[i].key == 's' && camera->keys_pressed[i].active)
				s = true;
		}

		camera->keys_pressed.clear();
		if (camera->go_to_breaking)
			computer->go_to_breaking();

		else if (s)
			computer->press_key('s');
	}
	
	if (blue_light)
	{
		chaos_frames++;
		if (chaos_frames % 3 == 0)
		{
			blue_frame_index++;
			if (blue_frame_index > 4)
				blue_frame_index = 1;

			monitor->textures[1] = "blueframe" + to_string(blue_frame_index) + ".png";
		}
	}
	
	if (counting_to_blue_light)
	{
		chaos_frames++;
		if (chaos_frames > 180)
		{
			chaos_frames = 0;
			blue_light = true;
			counting_to_blue_light = false;
			monitor->textures[1] = "blueframe1.png";
			computer->the_end = true;
			int max_index = 0;
			for (int i = 0; i < computer_setups.size(); ++i)
			{
				if (dist(camera->camera_x, camera->camera_z, computer_setups[i].x, computer_setups[i].z) >
					dist(camera->camera_x, camera->camera_z, computer_setups[max_index].x, computer_setups[max_index].z))
					max_index = i;

				computer_setup_index = max_index;
				draw_computer = true;
				current_light_x = computer_setups[max_index].x;
				current_light_y = 0.0;
				current_light_z = computer_setups[max_index].z;
				current_intensity = 3.0;
				camera->computer_present = true;
				camera->computer_x = current_light_x;
				camera->computer_z = current_light_z;
			}
		}
	}
	
	if (chaos_animation)
	{
		chaos_frames++;
		if (!sound_manager->is_playing("thesounds.wav"))
		{
			current_intensity = 0.0;
			draw_computer = false;
			chaos_animation = false;
			counting_to_blue_light = true;
			chaos_frames = 0;
		}

		else
		{
			if (chaos_frames % 120 == 0)
			{
				int max_index = 0;
				for (int i = 0; i < computer_setups.size(); ++i)
				{
					if (dist(camera->camera_x, camera->camera_z, computer_setups[i].x, computer_setups[i].z) >
						dist(camera->camera_x, camera->camera_z, computer_setups[max_index].x, computer_setups[max_index].z))
						max_index = i;

					computer_setup_index = max_index;
					draw_computer = true;
					current_light_x = computer_setups[max_index].x;
					current_light_y = 0.0;
					current_light_z = computer_setups[max_index].z;
					current_intensity = 3.0;
				}
			}

			else if ((chaos_frames - 10) % 120 == 0)
			{
				draw_computer = false;
				current_intensity = 0.0;
			}
		}
	}
	
	if (counting_down_to_chaos)
	{
		chaos_frames++;
		if (chaos_frames > 180)
		{
			chaos_frames = 0;
			counting_down_to_chaos = false;
			chaos_animation = true;
			current_intensity = 0.0;
			sound_manager->play_sound("thesounds.wav");
			float ambient[4] = { 0.0, 0.0, 0.0, 1.0 };
			glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
			glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.1);
		}
	}
	
	if (computer->playing_escape_sound)
	{
		if (sound_manager->is_playing(sound_manager->current_escape_sound))
		{
			if (camera->looked_behind || camera->camera_x <= 240)
			{
				computer->playing_escape_sound = false;
				sound_manager->smooth_stop(sound_manager->current_escape_sound);
				if (computer->score_table.back().second > 2200 && !computer->beat_space_game)
				{
					computer->websites["reddit"] = { "sleepy3.png" };
					computer->sequences["sequencereddit"] = { "decryptthread" };
					computer->day_number++;
					computer->submitted_poll_of_the_day = false;
					computer->beat_space_game = true;
					computer->whitelist.push_back("decryptthread");
					computer->delist("walter3");
					computer->delist("walter2");
					computer->delist("forumthreads");
					computer->delist("forumhome");
					computer->delist("thread11");
					computer->delist("thread12");
					computer->delist("thread21");
					computer->delist("thread22");
					computer->delist("thread31");
					computer->delist("thread32");
					computer->delist("thread41");
					computer->delist("thread51");
					computer->delist("thread52");
					computer->delist("game");
				}
				//draw_computer = false;
				//camera->computer_present = false;
				//counting_down_to_chaos = true;
				save_game(computer->save_slot);
			}
		}

		else reset_to_title();
	}
	
	if (lights_flickering)
		flicker_lights();
	
	if (game_state == STATE_AWAKENING)
	{
		animate_eyelids();
		if (!DEBUG)
		{
			if (!camera->do_file_animation)
				camera->start_file_animation("anim1");

			camera->animate();
		}
	}
	
	else if (game_state == STATE_NO_COMPUTER || game_state == STATE_WALKING || game_state == STATE_FLYING)
	{
		camera->animate();
		if (game_state == STATE_NO_COMPUTER)
		{
			if (camera->computer_ready())
			{
				game_state = STATE_WALKING;
				camera->computer_planes();
			}
		}

		if (camera->go_to_computer)
		{
			camera->go_to_computer = false;
			if (computer->read_email["rosecolored2bold.png"] && camera->went_to_corner)
			{
				if (!computer->looked_for_spider)
					computer->email_counter = 1;

				computer->looked_for_spider = true;
			}

			if (computer->read_email["rosecolored2bold.png"] && !camera->went_to_corner && !computer->looked_for_spider)
			{
				if (!computer->missed_spider)
					computer->email_counter = 1;

				computer->missed_spider = true;
			}

			camera->went_to_corner = false;
			computer->reset_computer();
			if (computer->computer_state == STATE_DESKTOP)
				computer->set_cursor("cursor.png");

			if (computer->computer_state == STATE_STARTUP)
				sound_manager->play_sound("startup.wav");

			game_state = STATE_COMPUTER;
		}

		if (wasd_alpha > 0.0 && wasd_alpha < 1.0)
			wasd_alpha += wasd_alpha_inc;
	}

	else if (game_state == STATE_COMPUTER)
	{
		computer->animate();
		if (computer->is_computer_done())
		{
			if (computer->computer_state == STATE_TITLE_SCREEN)
			{
				game_state = STATE_AWAKENING;
				computer->computer_state = STATE_STARTUP;
				if (continue_state != -1)
					computer->computer_state = STATE_DESKTOP;

				glutWarpPointer(100, 100);
				if (continue_state == 0)
					game_state = STATE_WALKING;

				if (continue_state == 1)
				{
					computer->reset_computer();
					game_state = STATE_COMPUTER;
				}
			}

			else
			{
				game_state = STATE_WALKING;
				glutWarpPointer(100, 100);
				camera->backing_up = true;
			}
		}

		if (computer->go_to_flying)
		{
			game_state = STATE_FLYING;
			computer->go_to_flying = false;
			camera->flying_mode = true;
			camera->rotating_quad = true;
			camera->camera_x = -900;
			camera->camera_y = 0;
			camera->camera_z = 0;
			camera->current_horiz_angle = PI / 2.0;
			camera->current_vert_angle = PI / 2.0;
			camera->press_key('w');
		}
	}

	else if (game_state == STATE_GETTING_UP)
	{
		animate_eyelids();
		if (DEBUG)
		{
			if (!camera->play_animations())
			{
				game_state = STATE_NO_COMPUTER;
				wasd_alpha += wasd_alpha_inc;
			}
		}

		else
		{
			camera->animate();
			if (!camera->do_file_animation)
			{
				game_state = STATE_NO_COMPUTER;
				wasd_alpha += wasd_alpha_inc;
			}
		}
	}

	else if (game_state == STATE_LOADING)
	{
		if (loading_percentage < 0.1)
		{
			loading_percentage += 0.01;
			if (loading_percentage >= 0.1)
				loading_stage++;
		}

		/*else
		{
			load_everything();
			loading_percentage = 1.0;
		}*/

		if (loading_stage == 1)
		{
			init_sound();
			loading_percentage = 0.2;
			loading_stage++;
		}

		else if (loading_stage == 2)
		{
			init_texture();
			loading_percentage = 0.7;
			loading_stage++;
		}

		else if (loading_stage == 3)
		{
			bulb->load_file("bulb2.obj");
			loading_percentage = 0.9;
			loading_stage++;
		}

		else if (loading_stage == 4)
		{
			monitor->load_file("computerjoined.obj");
			loading_percentage = 1.0;
			loading_stage++;
		}

		else if (loading_stage == 5)
		{
			computer->computer_state = STATE_TITLE_SCREEN;
			computer->reset_computer();
			game_state = STATE_COMPUTER;
		}
	}

	glutPostRedisplay();
	sound_manager->update_smooth_stop();
	sound_manager->release_sources();
	if (do_pause_menu)
	{
		if (pause_menu->pause_done)
		{
			do_pause_menu = false;
			if (game_state == STATE_COMPUTER)
				glutWarpPointer(computer->prev_mouse_x, 1080 - computer->prev_mouse_y);

			else glutWarpPointer(100, 100);
		}
	}

	glutTimerFunc(1000.0 / FRAMES_PER_SECOND, timer_callback, 0);
}

void load_everything()
{
	init_sound();
	init_texture();
	bulb->load_file("bulb2.obj");
	monitor->load_file("computerjoined.obj");
	game_state = STATE_AWAKENING;
}

void reset_game_state()
{
	elapsed_eyelid_time = 0;
	timebase = 0;
	continue_state = -1;
	eyelid_offset = 0.0;
	eyelid_velocity = 5.0;
	eyelid_accel = 0.0;
	look_vector_angle = PI / 2;
	look_vector_velocity = 0.0004;
	stage = 0;
	eyelid_wait_frames = 0;
	lights_flickering = false;
	current_intensity = 4.5;
	light_delta = -0.2;
	draw_computer = true;
	current_light_x = 0.0;
	current_light_y = WALL_HEIGHT - 81.0;
	current_light_z = 0.0;
	computer_setup_index = 0;
	counting_down_to_chaos = false;
	chaos_frames = 0;
	chaos_animation = false;
	blue_light = false;
	counting_to_blue_light = false;
	blue_frame_index = 1;
	wasd_alpha = 0.0;
	wasd_alpha_inc = 0.02;
	camera = make_unique<Camera>();
	camera->wall_planes();
	camera->sound_manager = sound_manager.get();
	camera->computer_x = 300;
	camera->computer_z = 0;
	computer = make_unique<Computer>(texture_manager.get());
	computer->sound_manager = sound_manager.get();
}

void reset_to_title()
{
	reset_game_state();
	computer->computer_state = STATE_TITLE_SCREEN;
	computer->reset_computer();
	computer->title_delay = 3 * 60;
	game_state = STATE_COMPUTER;
	reshape_callback(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
}

int main(int argc, char** argv)
{
	srand(time(NULL));
	computer_setups.push_back(Vector3(300, 180, 0));
	computer_setups.push_back(Vector3(-300, 0, 0));
	computer_setups.push_back(Vector3(0, 90, 300));
	computer_setups.push_back(Vector3(0, -90, -300));
	sound_manager = make_unique<SoundManager>();
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);
	glutInitWindowSize(INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT);
	glutInitWindowPosition(INITIAL_WINDOW_X, INITIAL_WINDOW_Y);
	glutCreateWindow("Ecce Simulacrum");
	glutReshapeFunc(reshape_callback);
	glutDisplayFunc(display_callback);
	glutKeyboardFunc(keyboard_callback);
	glutSpecialFunc(special_keyboard_callback);
	glutPassiveMotionFunc(passive_motion_callback);
	glutMotionFunc(passive_motion_callback);
	glutKeyboardUpFunc(keyboard_up_callback);
	glutSpecialUpFunc(special_keyboard_up_callback);
	glutMouseFunc(mouse_callback);
	glutTimerFunc(1000.0 / FRAMES_PER_SECOND, timer_callback, 0);
	glEnable(GL_DEPTH_TEST); //We'll probably need this...?
	texture_manager = make_unique<TextureManager>(2000); //TODO: THIS IS SILLY
	
	//texture_manager->test_save_image();
	//camera->planes.push_back(Plane(Vector3(-50, 200, -50), Vector3(1, 1, 1), 100, 0, 3));
	//camera->load_maze("maze1.txt");
	desk = make_unique<ObjModel>("table.obj", texture_manager.get());
	bulb = make_unique<ObjModel>("bulb2.obj", texture_manager.get());
	monitor = make_unique<ObjModel>("computer.obj", texture_manager.get());
	bulb->add_texture("black.png");
	bulb->add_texture("background2.png");
	bulb->add_texture("silver.png");
	desk->add_texture("silver.png");
	monitor->add_texture("white.png");
	monitor->add_texture("black.png");
	bulb->reserve(2448);
	desk->reserve(15746);
	monitor->reserve(41673);
	//desk->load_file("table.obj");
	
	pause_menu = make_unique<PauseMenu>(texture_manager.get());
	reset_game_state();
	init_light();
	glutFullScreen();
	glewInit();
	glewExperimental = true;
	init_fonts();
	string load_string = "";
	for (int i = 0; i < 128; ++i)
		load_string += i;

	draw_string(texture_manager.get(), 32, load_string, 0, 0, false, false, 0, false, true);
	draw_string(texture_manager.get(), 64, load_string, 0, 0, false, false, 0, false, true);
	draw_string(texture_manager.get(), 64, load_string, 0, 0, false, true, 0, false, true);
	draw_string(texture_manager.get(), 32, load_string, 0, 0, false, true, 0, false, true);
	change_font("LiberationMono-Bold.ttf");
	draw_string(texture_manager.get(), 32, load_string, 0, 0, false, true, 0, false, true);
	change_font("Audiowide-Regular.ttf");
	draw_string(texture_manager.get(), 32, load_string, 0, 0, false, true, 0, false, true);
	draw_string(texture_manager.get(), 90, load_string, 0, 0, false, true, 0, false, true);
	change_font("Lato-Regular.ttf");
	/*GLubyte* buffer = (GLubyte*)malloc(700 * 700 * 4 * sizeof(GLubyte));
	for (int i = 0; i < 700; ++i)
	{
		for (int j = 0; j < 700; ++j)
		{
			int start = j * 700 * 4 + i * 4;
			if (i >= 528)
			{
				buffer[start] = 0;
				buffer[start + 1] = 0;
				buffer[start + 2] = 0;
				buffer[start + 3] = 255;
			}

			else
			{
				buffer[start] = 3;
				buffer[start + 1] = 66;
				buffer[start + 2] = 79;
				buffer[start + 3] = 255;
			}
		}

		if (i < 528)
		{
			vector<int> choices;
			for (int k = 0; k < i; ++k)
			{
				bool done = false;
				int choice = 0;
				do
				{
					choice = rand() % 700;
					if (std::find(choices.begin(), choices.end(), choice) == choices.end())
						done = true;
				} while (!done);

				choices.push_back(choice);
				int start = choice * 700 * 4 + i * 4;
				if (rand() % 2 == 0)
				{
					buffer[start] = 150;
					buffer[start + 1] = 195;
					buffer[start + 2] = 25;
					buffer[start + 3] = 255;
				}

				else
				{
					buffer[start] = 143;
					buffer[start + 1] = 25;
					buffer[start + 2] = 194;
					buffer[start + 3] = 255;
				}
			}
		}
	}

	texture_manager->save_image("out.png", buffer, 700, 700);*/

	//_beginthread(load_everything, 0, NULL);
	texture_manager->load_texture("nowloading.png");
	texture_manager->load_texture("titleloading.png");
	texture_manager->load_texture("titleloadingfill.png");
	glutMainLoop();
}