#define _CRT_SECURE_NO_WARNINGS

//TODO check every srand to make sure no time_t
//TODO caps lock prevents you from moving

//#include <Windows.h>
//#include <mmsystem.h>
//#include <Vfw.h>
#include <time.h>
#include <sstream>
#include <memory>
#include "TextureManager.h"
#include "Camera.h"
#include "ObjModel.h"
#include "Computer.h"
#include "fonts.h"
#include "keymappings.h"

struct shadow_rect
{
	float x1, y1, x2, y2;
	float alpha;
	shadow_rect(float _x1, float _x2, float _y1, float _y2, float _alpha) : x1(_x1), x2(_x2), y1(_y1), y2(_y2), alpha(_alpha) {}
};

enum GameState {STATE_AWAKENING, STATE_NO_COMPUTER, STATE_WALKING, STATE_COMPUTER, STATE_PEEING, STATE_GETTING_UP, STATE_FLYING};
GameState game_state = STATE_AWAKENING;
std::unique_ptr<Camera> camera;
std::unique_ptr<TextureManager> texture_manager;
std::unique_ptr<ObjModel> desk;
std::unique_ptr<ObjModel> bulb;
std::unique_ptr<ObjModel> monitor;
std::unique_ptr<Computer> computer;

int eyelid_cycles;
int elapsed_eyelid_time = 0;
int timebase = 0;
std::vector<shadow_rect> rects;

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
//TEMPORARY

////// parameters for flickering the lights
bool lights_flickering = false;
float current_intensity = 4.5;
float light_delta = -0.2;
ALuint lightbulb_source;
//////

ALuint footsteps_source;
ALuint bomb_source;
ALuint title_source, main_source, game_over_source;
ALuint laser_source;

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
}

void init_sound()
{
	alGetError();
	ALCdevice *device;
	device = alcOpenDevice("Generic Software");
	ALCcontext *context;
	context = alcCreateContext(device, NULL);
	alcMakeContextCurrent(context);
	alGenSources((ALuint)1, &lightbulb_source);
	alGenSources((ALuint)1, &footsteps_source);
	alGenSources((ALuint)1, &bomb_source);
	alGenSources((ALuint)1, &title_source);
	alGenSources((ALuint)1, &main_source);
	alGenSources((ALuint)1, &game_over_source);
	alGenSources((ALuint)1, &laser_source);
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
	ALuint buffer, footsteps_buffer, bomb_buffer, title_buffer, main_buffer, game_over_buffer, laser_buffer;
	alGenBuffers((ALuint)1, &buffer);
	alGenBuffers((ALuint)1, &footsteps_buffer);
	alGenBuffers((ALuint)1, &bomb_buffer);
	alGenBuffers((ALuint)1, &title_buffer);
	alGenBuffers((ALuint)1, &main_buffer);
	alGenBuffers((ALuint)1, &game_over_buffer);
	alGenBuffers((ALuint)1, &laser_buffer);
	buffer_data_from_wav("light.wav", buffer);
	buffer_data_from_wav("step.wav", footsteps_buffer);
	buffer_data_from_wav("bomb.wav", bomb_buffer);
	buffer_data_from_wav("title.wav", title_buffer);
	buffer_data_from_wav("main.wav", main_buffer);
	buffer_data_from_wav("gameover.wav", game_over_buffer);
	buffer_data_from_wav("laser.wav", laser_buffer);
	alSourcei(lightbulb_source, AL_BUFFER, buffer);
	alSourcei(footsteps_source, AL_BUFFER, footsteps_buffer);
	alSourcei(bomb_source, AL_BUFFER, bomb_buffer);
	alSourcei(title_source, AL_BUFFER, title_buffer);
	alSourcei(main_source, AL_BUFFER, main_buffer);
	alSourcei(game_over_source, AL_BUFFER, game_over_buffer);
	alSourcei(laser_source, AL_BUFFER, laser_buffer);
}

void flicker_lights()
{
	if (current_intensity == 4.5)
		alSourcePlay(lightbulb_source);
	
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
		if (eyelid_wait_frames > 4.5 * 60)
		{
			eyelid_velocity = 4.0;
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
	texture_manager->load_texture("powersymbol1.png", true);
	texture_manager->load_texture("powersymbol2.png", true);
	texture_manager->load_texture("powersymbol3.png", true);
	texture_manager->load_texture("powersymbol4.png", true);
	texture_manager->load_texture("powersymbol5.png", true);
	texture_manager->load_texture("powersymbol6.png", true);
	texture_manager->load_texture("background.png");
	texture_manager->load_texture("bar.png", true);
	texture_manager->load_texture("test.png", true);
	texture_manager->load_texture("background2.png", true);
	texture_manager->load_texture("silver.png", true);
	texture_manager->load_texture("black.png", true);
	texture_manager->load_texture("light.png", true);
	texture_manager->load_texture("textfield.png", true);
	texture_manager->load_texture("xbutton.png", true);
	texture_manager->load_texture("search.png");
	texture_manager->load_texture("browserplan.png");
	texture_manager->load_texture("test2.png");
	texture_manager->load_texture("white.png", true);
    texture_manager->load_texture("icon_internet.png", true);
	texture_manager->load_texture("icon_internet_text.png", true);
    texture_manager->load_texture("arrow.png", true);
    texture_manager->load_texture("arrow2.png", true);
    texture_manager->load_texture("minimize.png", true);
    texture_manager->load_texture("back.png", true);
    texture_manager->load_texture("rfh1.png");
	texture_manager->load_texture("icon_folder.png", true);
	texture_manager->load_texture("icon_console.png", true);
	texture_manager->load_texture("beige.png");
	texture_manager->load_texture("beige2.png");
	texture_manager->load_texture("beigeold.png");
	texture_manager->load_texture("ok.png", true);
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
	texture_manager->load_texture("forward.png", true);
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
	texture_manager->load_texture("go.png", true);
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
	texture_manager->load_texture("contestdesc.png");
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
	texture_manager->load_texture("frame1.png", true);
	texture_manager->load_texture("frame2.png", true);
	texture_manager->load_texture("treesbackground.png");
	texture_manager->load_texture("river.png");
	texture_manager->load_texture("river1.png", true);
	texture_manager->load_texture("river2.png", true);
	texture_manager->load_texture("frame3.png", true);
	texture_manager->load_texture("frame4.png", true);
	texture_manager->load_texture("frame5.png", true);
	texture_manager->load_texture("frame6.png", true);
	texture_manager->load_texture("frame7.png", true);
	texture_manager->load_texture("frame8.png", true);
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
	texture_manager->load_texture("arrowthing.png", true);
	texture_manager->load_texture("smallarrow.png", true);
	texture_manager->load_texture("circle.png", true);
	texture_manager->load_texture("searchbar.png");
	texture_manager->load_texture("searchbutton.png");
	texture_manager->load_texture("helphome.png");
	texture_manager->load_texture("chatbutton.png");
	texture_manager->load_texture("forumhome.png");
	texture_manager->load_texture("header.png");
	texture_manager->load_texture("ellipse.png", true);
	texture_manager->load_texture("fill.png", true);
	texture_manager->load_texture("filledrectangle.png", true);
	texture_manager->load_texture("line.png", true);
	texture_manager->load_texture("paintbrush.png", true);
	texture_manager->load_texture("rectangle.png", true);
	texture_manager->load_texture("select.png", true);
	texture_manager->load_texture("stickylines.png", true);
	texture_manager->load_texture("save.png", true);
	texture_manager->load_texture("open.png", true);
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
	texture_manager->load_texture("browserok.png", true);
	texture_manager->load_texture("browserheld.png", true);
	texture_manager->load_texture("browserpressed.png", true);
	texture_manager->load_texture("invisible.png", true);
	texture_manager->load_texture("aincircle.png", true);
	texture_manager->load_texture("surveybackground.png", true);
	texture_manager->load_texture("radiobutton.png", true);
	texture_manager->load_texture("radiobuttonheld.png", true);
	texture_manager->load_texture("radiobuttonpushed.png", true);
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
	texture_manager->load_texture("xbuttonlight.png", true);
	texture_manager->load_texture("xbuttondark.png", true);
	texture_manager->load_texture("minimizelight.png", true);
	texture_manager->load_texture("minimizedark.png", true);
	texture_manager->load_texture("oklight.png", true);
	texture_manager->load_texture("okdark.png", true);
	texture_manager->load_texture("golight.png", true);
	texture_manager->load_texture("godark.png", true);
	texture_manager->load_texture("genlight.png", true);
	texture_manager->load_texture("gendark.png", true);
	texture_manager->load_texture("backdark.png", true);
	texture_manager->load_texture("backlight.png", true);
	texture_manager->load_texture("forwardlight.png", true);
	texture_manager->load_texture("forwarddark.png", true);
	texture_manager->load_texture("bookmarklight.png", true);
	texture_manager->load_texture("bookmarkdark.png", true);
	texture_manager->load_texture("searchbuttonflash.png", true);
	texture_manager->load_texture("result.png", true);
	texture_manager->load_texture("didyoulike.png", true);
	texture_manager->load_texture("bigwhite.png", true);
	texture_manager->load_texture("gameover.png");
	texture_manager->load_texture("icon_console_text.png", true);
	texture_manager->load_texture("icon_folder_text.png", true);
	texture_manager->load_texture("internet_name.png", true);
	texture_manager->load_texture("files_name.png", true);
	texture_manager->load_texture("console_name.png", true);
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
	texture_manager->load_texture("icon_paint_text.png", true);
	texture_manager->load_texture("paint_name.png", true);
	texture_manager->load_texture("scrollbackground.png", true);
	texture_manager->load_texture("nendatrash.png");
	texture_manager->load_texture("nendaspam.png");
	texture_manager->load_texture("nendaabout.png");
	texture_manager->load_texture("spinnerlight.png");
	texture_manager->load_texture("spinnerdark.png");
	texture_manager->load_texture("searchspinner.png");
	texture_manager->load_texture("couldntfind.png");
	texture_manager->load_texture("holder.png", true);
	texture_manager->load_texture("selectcolor.png", true);
	texture_manager->load_texture("paintbackground.png");
	texture_manager->load_texture("buttonblank.png", true);
	texture_manager->load_texture("colorpicker.png", true);
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
	texture_manager->load_texture("useful1.png");
	texture_manager->load_texture("highscoretable.png");
}

void init_light()
{
	GLfloat light_position[] = {0.0, static_cast<GLfloat>(WALL_HEIGHT - 81.0), 0.0, 1.0};
	GLfloat light_intensity[] = {current_intensity, current_intensity, current_intensity, 1.0}; //4.5
	glEnable(GL_LIGHTING);
	glEnable(GL_NORMALIZE);
	glShadeModel(GL_SMOOTH);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_intensity);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_intensity);
	glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 5.0); //8?
	glEnable(GL_LIGHT0);
}

void reshape_callback(int width, int height)
{
	glViewport(0, 0, width, height);
	camera->window_resized(width, height);
	computer->window_resized(width, height);
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
	for (int i = 0; i < WALL_QUADS_HORIZ; ++i)
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
	}

	/*glTranslatef(-WALL_DISTANCE * 2, 0.0, 0.1);
	for (int i = 0; i < WALL_TEXTURE_S; ++i)
	{
		glDisable(GL_LIGHTING);
		glLineWidth(2.0);
		glBegin(GL_LINES);
			glColor4f(0.0, 0.0, 0.0, 1.0);
			glVertex3f(0.0, 0.0, 0.0);
			glVertex3f(0.0, WALL_HEIGHT, 0.0);
		glEnd();
		glTranslatef((WALL_DISTANCE*2)/WALL_TEXTURE_S, 0.0, 0.0);
		glEnable(GL_LIGHTING);
	}*/

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
	for (int i = 0; i < FLOOR_QUADS_HORIZ; ++i)
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
	}

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
	
	texture_manager->change_texture("tiles.bmp");
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
	
	draw_floor(false);
	//draw_beads();

	light_position[1] = WALL_HEIGHT * -2.0;
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);

	if (game_state != STATE_FLYING)
	{
		glPushMatrix();
		glTranslatef(0.0, WALL_HEIGHT, 0.0);
		glRotatef(180.0, 0.0, 0.0, 1.0);
		draw_floor(true);
		glPopMatrix();
	}

	draw_bulb();

	if (game_state != STATE_NO_COMPUTER)
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
	float emission[4] = { 0.4, 0.4, 0.4, 1.0 };
	float default_emission[4] = { 0.0, 0.0, 0.0, 1.0 };
	texture_manager->change_texture("edges.bmp");
	glPushMatrix();
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
		glTranslatef(300.0, 32.0, 0.0);
		glScalef(20.0, 20.0, 20.0);
		//desk->draw();
		glTranslatef(0.0, 1.0, 0.0);
		glRotatef(180.0, 0.0, 1.0, 0.0);
		monitor->draw();
		glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, default_emission);
	glPopMatrix();
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
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	//glDepthMask(GL_FALSE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBegin(GL_QUADS);
		/*for (int i = 0; i < rects.size(); ++i)
		{
			glColor4f(0.0, 0.0, 0.0, rects[i].alpha);
			glRectf(rects[i].x1, rects[i].y1, rects[i].x2, rects[i].y2);
		}*/
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
	glTranslatef(0.0, eyelid_offset, 0.0);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.99); glVertex2f(0.0, 0.0);
		glTexCoord2f(0.0, 0.0); glVertex2f(0.0, height);
		glTexCoord2f(1.0, 0.0); glVertex2f(width, height);
		glTexCoord2f(1.0, 0.99); glVertex2f(width, 0.0);
	glEnd();
	/*glColor4f(0.0, 0.0, 0.0, 1.0);
		glVertex3f(0.0, 0.0, 0.0);
		glVertex3f(width, 0.0, 0.0);
		glVertex3f(width, eyelid_pos, 0.0);
		glVertex3f(0.0, eyelid_pos, 0.0);
		for (int i = 0; i < 10; ++i)
		{
			glColor4f(0.0, 0.0, 0.0, 1.0 - ((float) i / 10.0) - 0.1);
			float new_pos = eyelid_pos + i*5;
			glVertex3f(0.0, new_pos, -0.3);
			glVertex3f(width, new_pos, -0.3);
			glVertex3f(width, new_pos + 5.0, -0.3);
			glVertex3f(0.0, new_pos + 5.0, -0.3);
		}

		glColor4f(0.0, 0.0, 0.0, 1.0);
		float mid = (float) height / 2.0;
		float new_eyelid_pos = mid + (mid - eyelid_pos);
		glVertex3f(0.0, new_eyelid_pos, 0.0);
		glVertex3f(width, new_eyelid_pos, 0.0);
		glVertex3f(width, height, 0.0);
		glVertex3f(0.0, height, 0.0);
		glColor4f(0.0, 0.0, 0.0, eyelid_pos / mid);
		glVertex3f(0.0, 0.0, -0.5);
		glVertex3f(width, 0.0, -0.5);
		glVertex3f(width, height, -0.5);
		glVertex3f(0.0, height, -0.5);*/
	//glEnd();
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glPopMatrix();
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

		draw_room();
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

		/*new_frames++;
		end_t = time(NULL);
		if (end_t - start > 0)
		{
			fps = new_frames / (end_t - start);
			new_frames = 0;
		}

		std::stringstream ss;
		ss << fps;
		glEnable(GL_TEXTURE_2D);
		draw_string(texture_manager.get(), 32, ss.str(), 5.0, 800.0);
		cout << ss.str() << endl;*/
	}

	if (game_state == STATE_AWAKENING || game_state == STATE_GETTING_UP)
	{
		draw_eyelids();
	}

	if (game_state == STATE_FLYING)
	{
		glDisable(GL_LIGHTING);
		if (camera->rotating_quad)
		{
			texture_manager->change_texture("red.png");
			float ydist = tan((50.0 / 2.0) * PI / 180) * 2;
			float zdist = ydist * camera->aspect;
			glPushMatrix();
			glTranslatef(-898, -ydist, 0.0);
			glRotatef(camera->rotation_angle, 0.0, 0.0, -1.0);
			glBegin(GL_QUADS);
			/*glTexCoord2f(0.0, 0.0); glVertex3f(-898, -ydist, -zdist);
			glTexCoord2f(0, 1); glVertex3f(-898, -ydist, zdist);
			glTexCoord2f(1, 1); glVertex3f(-898, ydist, zdist);
			glTexCoord2f(1, 0); glVertex3f(-898, ydist, -zdist);*/
			glTexCoord2f(0.0, 0.0); glVertex3f(0, 0, -zdist);
			glTexCoord2f(0, 1); glVertex3f(0, 0, zdist);
			glTexCoord2f(1, 1); glVertex3f(0, ydist*2, zdist);
			glTexCoord2f(1, 0); glVertex3f(0, ydist*2, -zdist);
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
		glDisable(GL_DEPTH_TEST);
		texture_manager->change_texture("silver.png");
		glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex2f(0.0, 1000);
			glTexCoord2f(0.0, 1); glVertex2f(0.0, 1080);
			glTexCoord2f(1.0, 1); glVertex2f(1920, 1080);
			glTexCoord2f(1.0, 0.0); glVertex2f(1920, 1000);
		glEnd();
		texture_manager->change_texture("blue.png");
		glBegin(GL_QUADS);
			glTexCoord2f(0.0, 0.0); glVertex2f(0.0, 0);
			glTexCoord2f(0.0, 1); glVertex2f(0.0, 56);
			glTexCoord2f(1.0, 1); glVertex2f(1920, 56);
			glTexCoord2f(1.0, 0.0); glVertex2f(1920, 0);
		glEnd();
		glEnable(GL_DEPTH_TEST);
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
	}

	glDisable(GL_TEXTURE_2D);
	glutSwapBuffers();
	//glDepthMask(GL_TRUE);
	//glEnable(GL_DEPTH_TEST);
}

void keyboard_callback(unsigned char key, int x, int y)
{
	if (key == 27)
		exit(0);

	if (key == 'n' && game_state == STATE_AWAKENING || game_state == STATE_GETTING_UP)
	{
		game_state = STATE_WALKING;
		camera->camera_y = INITIAL_WALKING_Y;
		camera->look_x = 0.0;
		camera->look_y = 1.0;
		camera->look_z = 0.0;
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

	if (game_state == STATE_NO_COMPUTER || game_state == STATE_WALKING || game_state == STATE_FLYING)
		camera->press_key(key);

	else if (game_state == STATE_COMPUTER)
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
	if (game_state == STATE_NO_COMPUTER || game_state == STATE_WALKING || game_state == STATE_FLYING)
	{
		camera->mouse_moved(x, y);
		glutPostRedisplay();
	}

	if (game_state == STATE_COMPUTER)
	{
		computer->mouse_moved(x, y);
		glutPostRedisplay();
	}
}

void mouse_callback(int button, int state, int x, int y)
{
	cout << to_string(x) + ", " + to_string(glutGet(GLUT_WINDOW_HEIGHT) - y) + "\n";
	if (game_state == STATE_COMPUTER)
		computer->mouse_clicked(button, state, x, y);
}

void timer_callback(int value)
{
	if (lights_flickering)
		flicker_lights();
	
	if (game_state == STATE_AWAKENING)
		animate_eyelids();
	
	else if (game_state == STATE_NO_COMPUTER || game_state == STATE_WALKING || game_state == STATE_FLYING)
	{
		camera->animate();
		if (game_state == STATE_NO_COMPUTER)
		{
			if (camera->computer_ready())
				game_state = STATE_WALKING;
		}

		if (camera->go_to_computer)
		{
			camera->go_to_computer = false;
			if (computer->read_email["rosecolored2bold.png"] && camera->went_to_corner)
				computer->looked_for_spider = true;

			/*if (computer->read_email["rosecolored2bold.png"] && !camera->went_to_corner && !computer->looked_for_spider)
				computer->missed_spider = true;*/ //TODO put this back in place

			camera->went_to_corner = false;
			computer->reset_computer();
			if (computer->computer_state == STATE_DESKTOP)
				glutSetCursor(GLUT_CURSOR_LEFT_ARROW);

			game_state = STATE_COMPUTER;
		}
	}

	else if (game_state == STATE_COMPUTER)
	{
		computer->animate();
		if (computer->is_computer_done())
		{
			game_state = STATE_WALKING;
			glutWarpPointer(100, 100);
			camera->backing_up = true;
		}

		if (computer->go_to_flying)
		{
			game_state = STATE_FLYING;
			camera->flying_mode = true;
			camera->rotating_quad = true;
			camera->camera_x = -900;
			camera->camera_y = 0;
			camera->camera_z = 0;
			camera->current_horiz_angle = PI / 2.0;
			camera->current_vert_angle = PI / 2.0;
		}
	}

	else if (game_state == STATE_GETTING_UP)
	{
		animate_eyelids();
		if (!camera->play_animations())
			game_state = STATE_WALKING;
	}

	glutPostRedisplay();
	glutTimerFunc(1000.0 / FRAMES_PER_SECOND, timer_callback, 0);
}

int main(int argc, char** argv)
{
	srand(time(NULL));
	init_sound();
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
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
	texture_manager = make_unique<TextureManager>(1000); //TODO: THIS IS SILLY
	init_texture();
	camera = make_unique<Camera>();
	camera->footsteps_source = footsteps_source;
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
	bulb->load_file("bulb2.obj");
	monitor->load_file("computerjoined.obj");
	computer = make_unique<Computer>(texture_manager.get());
	computer->sound_sources.push_back(footsteps_source);
	computer->sound_sources.push_back(bomb_source);
	computer->sound_sources.push_back(title_source);
	computer->sound_sources.push_back(main_source);
	computer->sound_sources.push_back(game_over_source);
	computer->sound_sources.push_back(laser_source);
	init_light();
	glutFullScreen();
	glewInit();
	glewExperimental = true;
	//init_shaders();
	init_fonts();
	glutMainLoop();
}