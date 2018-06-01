#define _CRT_SECURE_NO_WARNINGS

#include <algorithm>
#include <thread>
#include <chrono>
#include <filesystem>
#include <windows.h>
#include "Computer.h"
#include "PowerSymbol.h"
#include "TextField.h"
#include "DesktopIcon.h"
#include "XButton.h"
#include "GameParams.h"
#include "fonts.h"
#include "ScrollPanel.h"
#include "ScrollBar.h"
#include "Button.h"
#include "Files.h"
#include "Console.h"
#include "InfoText.h"
#include "BubbleGame.h"
#include "Canvas.h"
#include "Resizer.h"
#include "MMO.h"
#include "HelpCenter.h"
#include "NumberField.h"
#include "Video.h"
#include "ProgressBar.h"
#include "Decryption.h"
#include "LevelEditor.h"
#include "BrokenThing.h"
#include "RotatingBox.h"
#include "DragElement.h"
#include "ShoppingCenter.h"
#include "BackgroundAnimation.h"
#include "Animation.h"
#include "keymappings.h"
#include "binaryio.h"

using namespace std;

void load_game(string filename);
void save_game(string filename);

//TODO Can't allow user to resize game window, otherwise TERRIBLE THINGS WILL HAPPEN (please fix that)
//TODO clicking on things while forcing_cursor?
//TODO up click on different element should not take focus?  Destroys highlighting in text box.
//TODO click on a link to a page that loads children, minimize while page is loading, minimization animation won't play anymore, happens very fast (related to adding children while minimized?)
//(doesn't seem to be happening now... test with long page load times to verify)
//TODO can separate cursor from window being dragged by moving very fast into tray
//TODO "Internet" text at top of browser is kind of blurry
//TODO tray icon doesn't play animation once breaking starts in earnest

float scalex(float x)
{
	return x * ((float)glutGet(GLUT_WINDOW_WIDTH) / 1920.0);
}

float scaley(float y)
{
	return y * ((float)glutGet(GLUT_WINDOW_HEIGHT) / 1080.0);
}

float unscalex(float x)
{
	return x * (1920.0 / (float)glutGet(GLUT_WINDOW_WIDTH));
}

float unscaley(float y)
{
	return y * (1080.0 / (float)glutGet(GLUT_WINDOW_HEIGHT));
}

bool intersects(ScreenElement s1, ScreenElement s2)
{
	return (s1.x1 < s2.x2 && s1.x2 > s2.x1 &&
		s1.y1 < s2.y2 && s1.y2 > s2.y1);
}

Computer::Computer(TextureManager* manager)
{
	required_target = "";
	title_delay = 0;
	uploaded_image.image = NULL;
	dialogue_confirm_function = []() {};
	restore_application = NONE;
	save_slot = "savedata1";
	websites["search"] = { "search.png" };
	websites["reddit"] = { "sleepy1.png" };
	websites["tablet"] = { "sleepytablet.png" };
	websites["conversation"] = { "sleepyconversation.png" };
	websites["tomthread"] = { "sleepytom.png" };
	websites["useful"] = { "useful.png" };
	websites["nenda"] = { "nendablank.png" };
	websites["rfh2"] = { "rfh2.png" };
	websites["rfh1"] = { "rfh1.png" };
	websites["rfh5"] = { "rfh5.png" };
	websites["testwebsite"] = { "nendablank.png" };
	websites["deliverfailure"] = { "deliverfailure.png" };
	websites["forumemail1"] = { "forumemail1.png" };
	websites["forumemail2"] = { "forumemail2.png" };
	websites["error"] = { "errorpage.png" };
	websites["rosecolored1"] = { "rosecolored1.png" };
	websites["rosecolored2"] = { "rosecolored2.png" };
	websites["rosecolored3"] = { "rosecolored3.png" };
	websites["rosecolored4"] = { "rosecolored4.png" };
	websites["rosecolored5"] = { "rosecolored5.png" };
	websites["rosecoloredmissedspider"] = { "rosecoloredmissedspider.png" };
	websites["searchresults"] = { "searchresults.png" };
	websites["helpcenter"] = { "bigwhite.png" };
	websites["survey"] = { "surveybackground.png" };
	websites["survey2"] = { "surveybackground.png" };
	websites["survey3"] = { "surveybackground.png" };
	websites["survey4"] = { "surveybackground.png" };
	websites["ain"] = { "ain1.png" };
	websites["e1"] = { "e1.png" };
	websites["psych"] = { "psych.png" };
	websites["thankyouforuploading"] = { "thankyouforuploading.png" };
	websites["centerracom"] = { "centerracomhomepage.png" };
	websites["centerracomtrial"] = { "centerracomtrial.png" };
	websites["centerracomthankyou"] = { "centerracomthankyou.png" };
	websites["triallink"] = { "triallink.png" };
	websites["surveylink"] = { "surveylink.png" };
	websites["hammercounters1"] = { "e1.png" };
	websites["decryptpage"] = { "decryption.png" };
	websites["e2"] = { "e1.png" };
	websites["helphome"] = { "helphome.png" };
	websites["forumhome"] = { "forumhome.png" };
	websites["contestresults"] = { "sleepy.png" };
	websites["game"] = { "bigwhite.png" };
	websites["---"] = { "thankyouforuploading.png" };
	websites["mmo"] = { "e1.png" };
	websites["recoveremail"] = { "recoveremail.png" };
	websites["recoverpassword"] = { "e1.png" };
	websites["psychologist"] = { "psychologist.png" };
	websites["spam"] = { "nendaspam.png" };
	websites["trash"] = { "nendatrash.png" };
	websites["about"] = { "nendaabout.png" };
	websites["search/search?q="] = { "defaultresults.png" };
	websites["break"] = { "breakpage.png" }; //{ "search.png" };
	websites["tom1"] = { "tom1.png" };
	websites["tom2"] = { "tom2.png" };
	websites["tom3"] = { "tom3.png" };
	websites["funcorner"] = { "funcorner.png" };
	websites["catalogue"] = { "invisible.png" };
	websites["itemstore"] = { "invisible.png" };
	websites["cart"] = { "invisible.png" };
	websites["confirmation"] = { "invisible.png" };
	websites["purchase"] = { "purchase.png" };
	websites["errorpurchase"] = { "invisible.png" };
	websites["thankyou"] = { "invisible.png" };
	websites["messages"] = { "invisible.png" };
	websites["ownership"] = { "invisible.png" };
	websites["purchasecomplete"] = { "purchasecomplete.png" };
	websites["advertising"] = { "advertising.png" };
	websites["arcade"] = { "paul.png" };
	websites["centerracomcontact"] = { "centerracomcontact.png" };
	websites["centerracompartners"] = { "centerracompartners.png" };
	websites["testthread"] = { "testthread.png" };
	websites["forumthreads"] = { "forumthreads.png" };
	websites["rfh3"] = { "rfh3.png" };
	websites["rfh4"] = { "rfh4.png" };
	websites["thread11"] = { "thread11.png" };
	websites["thread12"] = { "thread12.png" };
	websites["thread21"] = { "thread21.png" };
	websites["thread22"] = { "thread22.png" };
	websites["thread31"] = { "thread31.png" };
	websites["thread32"] = { "thread32.png" };
	websites["thread41"] = { "thread41.png" };
	websites["thread51"] = { "thread51.png" };
	websites["thread52"] = { "thread52.png" };
	websites["hcthread"] = { "hcthread.png" };
	websites["imagehost"] = { "imagehost.png" };
	websites["rosecolored6"] = { "rosecolored6.png" };
	websites["rfh6"] = { "rfh6.png" };
	websites["adminnotice"] = { "adminnotice.png" };
	websites["linesexist"] = { "linesexist.png" };
	websites["squaresexist"] = { "squaresexist.png" };
	websites["brandonexist"] = { "brandonexist.png" };
	websites["existconquest"] = { "existconquest.png" };
	websites["deliveryexist"] = { "deliveryexist.png" };
	websites["typingexist"] = { "typingexist.png" };
	websites["eula"] = { "eula.png" };
	websites["contestupload"] = { "upload.png" };
	websites["walter1"] = { "walter1.png" };
	websites["walter2"] = { "walter2.png" };
	websites["centerracominvalid"] = { "centerracominvalid.png" };
	websites["walter3"] = { "walter3.png" };
	websites["decryptthread"] = { "decryptthread.png" };
	websites["rfh7"] = { "rfh7.png" };

	sequences["sequencereddit"] = { "tomthread", "tablet", "conversation" };
	sequences["sequenceforum"] = { "thread51", "thread41", "thread31", "thread21", "thread11" };
	sequences["sequenceain"] = { "e1" };
	
	cursor = "cursor.png";
	day_number = 1;
	score_table.push_back(pair<string, int>("JIM", 1));
	score_table.push_back(pair<string, int>("JIM", 2));
	score_table.push_back(pair<string, int>("JIM", 3));
	score_table.push_back(pair<string, int>("JIM", 4));
	score_table.push_back(pair<string, int>("JIM", 5));
	score_table.push_back(pair<string, int>("JIM", 6));
	score_table.push_back(pair<string, int>("JIM", 1300));
	score_table.push_back(pair<string, int>("JIM", 1500));
	score_table.push_back(pair<string, int>("JIM", 2000));
	score_table.push_back(pair<string, int>("JIM", 2200));
	no_mouse = false;
	queue_click_x = -1;
	queue_click_y = -1;
	flash_counter = -1;
	focus_application = NONE;
	minimize_bucket = close_bucket = url_bucket = scroll_bucket = power_bucket = tray_bucket = 0;
	decrypt_forcing = false;
	whitelist = { "search", "reddit", "ain", "nenda", "useful", "e1", "advertising", "purchase", "conversation", "tablet", "tomthread", "deliverfailure", "forumemail1", "forumemail2", "rfh1", "rfh2" };
	mmo_password = "a";
	applications_open = 0;
	char buffer[100];
	focus_element = NULL;
	if (DEBUG)
		computer_state = STATE_DESKTOP;

	else computer_state = STATE_STARTUP;
	emails = { "deliverfailurebold.png", "forumemail2bold.png", "forumemail1bold.png", "rfh2bold.png", "rfh1bold.png" };
	for (int i = 0; i < emails.size(); ++i)
		read_email[emails[i]] = false;

	current_wait = 0;
	prev_time = 0;
	alpha = 1.0;
	alpha_increment = -ALPHA_INCREMENT;
	computer_done = false;
	going_to_sleep = false;
	texture_manager = manager;
	messages.push_back(Message("EXF Boot Agent v2.07.09", 300));
	messages.push_back(Message("Checking dev0... [OK]", 1));
	messages.push_back(Message("Checking DDRS... [OK]", 1));
	messages.push_back(Message("Calibrating eye tracking...", 3000));
	messages.push_back(Message("Loading kernel drivers", 200));
	messages.push_back(Message("ERROR:  Bad boot sector:  Recompiling module HBA0...", 1));
	for (int i = 1; i <= 21; ++i)
	{
		sprintf(buffer, "mixgc hba%d.c -o hba%d.o -l ldd -ICC:/dev/0/init -Floor -cxxnc -clo --sector-output 00A", i, i);
		messages.push_back(Message(string(buffer), 333));
	}

	messages.push_back(Message("mv hba0part0 /dev/0/init", 100));
	messages.push_back(Message("mv hba0part1 /dev/0/init", 100));
	messages.push_back(Message("Loading module HBA1... [OK]", 100));
	messages.push_back(Message("Display driver v7.01", 100));
	messages.push_back(Message("Initializing color matrix... [OK]", 100));
	messages.push_back(Message("Setting EE bump maps... [OK]", 100));
	messages.push_back(Message("Checking stereoscopic display... [FAILURE]", 100));
	messages.push_back(Message("Importing view scan 60Hz... [OK]", 100));
	messages.push_back(Message("Static Ethernet PXN compatible ee6.51.7", 100));
	messages.push_back(Message("Waiting for DHCP response...", 3000));
	messages.push_back(Message("DHCP OK 127.0.9.255", 100));
	messages.push_back(Message("Kernel modules initialized", 500));
	messages.push_back(Message("Loading desktop...", 2000));

	folder_names = { "users", "xj5555585", "documents", "pics", "oldstuff", "random", "stage1", "stopit", "systemdata", "goaway", "imeanit", "q", "" };

	//PUT FLAG OVERRIDES HERE
	//read_email["rosecolored4bold.png"] = true;
	//update_in_progress = false;
	//day_number = 5;
	//entered_contest = true;
	//ran_trial = true;
	//looked_for_spider = true;
	//tried_to_view_encrypted_folder = true;
	//reached_psychologist = true;
	//entered_contest = true;
	//looked_for_spider = true;
	//END OVERRIDES
}

void Computer::display_pause_screen()
{
	/*auto black = make_unique<ScreenElement>(0, 0, 1920, 1080, "black.png");
	black->alpha = 0.7;
	screen_elements.push_back(std::move(black));
	screen_elements.push_back(make_unique<ScreenElement>(880, 1080 - 402, -1, -1, "doyouwanttoquit.png"));
	screen_elements.push_back(make_unique<Button>(880, 1080 - 447, -1, -1, "yes1.png", NONE, []() {exit(0);}, []() {}, "", "yes2.png", "yes2.png", "", false, true));
	screen_elements.push_back(make_unique<Button>(999, 1080 - 447, -1, -1, "no1.png", NONE, [this]()
	{
		for (int i = screen_elements.size() - 1; i >= screen_elements.size() - 4; --i)
			screen_elements[i]->marked_for_deletion = true;
	}, []() {}, "", "no2.png", "no2.png", "", false, true));*/
}

string Computer::get_path_name(string folder_name)
{
	if (folder_name == "/")
		folder_name = "root";

	string path_name = save_slot + "p\\" + folder_name + "\\";
	CreateDirectory(path_name.c_str(), NULL);
	return path_name;
}

bool Computer::files_present()
{
	for (int i = 0; i < folder_names.size() - 1; ++i)
	{
		if (get_folder_index(folder_names[i]).size() != 0)
			return true;
	}

	return get_folder_index("root").size() != 0;
}

File Computer::read_image_file(string file_name, string folder_name)
{
	File file;
	ifstream in(get_path_name(folder_name) + file_name, ios::binary);
	if (in.good())
		file = read_binary<File>(in);

	in.close();
	return file;
}

vector<File> Computer::get_folder_index(string folder_name)
{
	vector<File> index;
	for (auto& p : std::experimental::filesystem::directory_iterator(get_path_name(folder_name)))
	{
		ifstream in(p.path().string(), ios::binary);
		File file;
		file.name = read_binary<string>(in);
		file.type = read_binary<int>(in);
		index.push_back(file);
		in.close();
	}

	return index;
}

int Computer::get_folder_size(string folder_name)
{
	int size = 0;
	for (auto& p : std::experimental::filesystem::directory_iterator(get_path_name(folder_name)))
		size++;

	return size;
}

bool Computer::write_file_to_folder(File file, string folder_name)
{
	ofstream out(get_path_name(folder_name) + file.name, ios::binary);
	if (!out.good())
		return false;

	write_binary(file, out);
	out.close();
	return true;
}

void Computer::delete_file(string file_name, string folder_name)
{
	string name = get_path_name(folder_name) + file_name;
	remove(name.c_str());
}

void Computer::delist(string website)
{
	websites.erase(website);
	whitelist.erase(remove_if(whitelist.begin(), whitelist.end(), [website](string str) {return str == website;}), whitelist.end());
}

void Computer::increment_breaking_stage()
{
	breaking_stage++;
	if (breaking_stage == 1)
	{
		int j = 0;
		for (j = 0; j < screen_elements.size(); ++j)
		{
			if (dynamic_cast<ScrollPanel*>(screen_elements[j].get()))
				break;
		}

		auto power_button = screen_elements[screen_elements.size() - 2].get();
		power_button->is_visible = false;
		auto new_button = make_unique<BrokenThing>(power_button->x1 + 7, power_button->y1, power_button->x2, power_button->y2, "powersymbol.png", dynamic_cast<ScrollPanel*>(screen_elements[j].get()));
		new_button->fake = true;
		screen_elements.push_back(std::move(new_button));
	}

	if (breaking_stage == 3)
	{
		int j = 0;
		TextField* text_field = NULL;
		for (j = 0; j < screen_elements.size(); ++j)
		{
			if (text_field = dynamic_cast<TextField*>(screen_elements[j].get()))
				break;
		}

		text_field->is_visible = false;
		auto rotating_box = make_unique<RotatingBox>(text_field->x1, text_field->y1, text_field->x2, text_field->y2, "textfield.png");
		rotating_box->angle = 0;
		rotating_box->parent = this;
		rotating_box->prepare_string(text_field->text);
		rotating_box->fake = true;
		screen_elements.push_back(std::move(rotating_box));
	}

	if (breaking_stage == 4)
	{
		make_fake(Rect(0, 1000, 1920, 1080));
	}

	if (breaking_stage == 5)
	{
		make_fake(Rect(1900.0, 56.0, 1920.0, 1080), false);
	}

	if (breaking_stage == 6)
	{
		make_fake(Rect(0, 0, 1920, 56));
	}
}

void Computer::make_fake(Rect region, bool do_black)
{
	if (do_black)
		screen_elements.push_back(make_unique<ScreenElement>(region.x1, region.y1, region.x2, region.y2, "black.png"));

	for (int i = 0; i < screen_elements.size(); ++i)
	{
		ScreenElement* se = screen_elements[i].get();
		if (se->x1 >= region.x1 && se->x2 <= region.x2 && se->y1 >= region.y1 && se->y2 <= region.y2 && !se->fake && se->name != "black.png")
		{
			se->is_visible = false;
			faked_out.push_back(se);
			auto fake_element = make_unique<ScreenElement>(se->x1, se->y1, se->x2, se->y2, se->name);
			fake_element->fake = true;
			screen_elements.push_back(std::move(fake_element));
		}
	}

	have_active_fake = true;
	active_fake = region;
}

void Computer::move_fake()
{
	bool found = false;
	for (int i = 0; i < screen_elements.size(); ++i)
	{
		ScreenElement* se = screen_elements[i].get();
		if (se->fake && se->y2 > -1000) //TODO so things don't stick up
		{
			se->translate(0, -20);
			se->angle += 2;
			found = true;
		}
	}

	if (!found)
	{
		animate_fake = false;
		have_active_fake = false;
		increment_breaking_stage();
	}
}

void Computer::restore_browser()
{
	everything_stuck = false;
	int scroll_index = 0;
	for (int i = 0; i < faked_out.size(); ++i)
		faked_out[i]->is_visible = true;

	for (int i = 0; i < screen_elements.size(); ++i)
	{
		if (dynamic_cast<ScrollPanel*>(screen_elements[i].get()))
			scroll_index = i;

		if (screen_elements[i]->fake || screen_elements[i]->name == "black.png")
			screen_elements[i]->marked_for_deletion = true;
	}

	ScrollPanel* scroll_panel = dynamic_cast<ScrollPanel*>(screen_elements[scroll_index].get());
	scroll_panel->change_website("search");
	process_control_changes();
}

void Computer::go_to_breaking()
{
	ScrollPanel* scroll_panel = NULL;
	for (int j = 0; j < screen_elements.size(); ++j)
	{
		if (scroll_panel = dynamic_cast<ScrollPanel*>(screen_elements[j].get()))
			break;
	}

	scroll_panel->change_website("break");
}

string Computer::get_cur_dir_name(int index)
{
	if (index == 0)
		return "/";

	string name = "";
	for (int i = 0; i < index; ++i)
		name += "/" + folder_names[i];

	return name;
}

void Computer::window_resized(int new_width, int new_height)
{
	width = new_width;
	height = new_height;
	grid_width = width / DENIAL_CHAR_WIDTH * 2;
	grid_height = height / DENIAL_CHAR_HEIGHT * 2;
}

void Computer::setup_view() //Depth test?
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0.0, width, 0.0, height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); //Do we need to do this?
}

void Computer::draw_messages(float xtrans, float ytrans, float scale)
{
	//Presently relying on the fact that display_callback() clears the screen to black
	glDisable(GL_LIGHTING);
	for (int i = 0; i < char_grid.size(); ++i)
	{
		for (int j = 0; j < char_grid[i].size(); ++j)
		{
			texture_manager->change_texture("font.bmp");
			int absolute_pos = 0;
			char current_char = char_grid[i][j];
			if ('A' <= current_char && current_char <= 'Z')
				absolute_pos = current_char - 'A';

			else if ('a' <= current_char && current_char <= 'z')
				absolute_pos = current_char - 'a' + 26;

			else if ('0' <= current_char && current_char <= '9')
				absolute_pos = current_char - '0' + 52;

			else if (current_char == '.')
				absolute_pos = 62;

			else if (current_char == '\'')
				absolute_pos = 69;

			else if (current_char == '_')
				absolute_pos = 73;

			else if (current_char == '(')
				absolute_pos = 79;

			else if (current_char == ')')
				absolute_pos = 80;

			else if (current_char == '[')
				absolute_pos = 89;

			else if (current_char == ']')
				absolute_pos = 90;

			else if (current_char == ':')
				absolute_pos = 84;

			else if (current_char == '-')
				absolute_pos = 82;

			else if (current_char == '/')
				absolute_pos = 83;

			else if (current_char == '>')
				absolute_pos = 88;

			else if (current_char == -1)
				absolute_pos = 90;

			int y_coord = 6 - (absolute_pos / 14);
			int x_coord = absolute_pos % 14;
			float s1 = (float) (DENIAL_WIDTH_OFFSET + x_coord * DENIAL_CHAR_WIDTH) / 468.0;
			float t1 = (float) (y_coord * DENIAL_CHAR_HEIGHT) / 465.0;
			float s2 = (float) (DENIAL_WIDTH_OFFSET + (x_coord + 1) * DENIAL_CHAR_WIDTH) / 468.0;
			float t2 = (float) ((y_coord + 1) * DENIAL_CHAR_HEIGHT) / 465.0;
			float x1 = (float) (j * DENIAL_CHAR_WIDTH * scale);
			float y1 = (float) (height - (DENIAL_CHAR_HEIGHT * scale * (i + 1)));
			float x2 = (float) ((j + 1) * DENIAL_CHAR_WIDTH * scale);
			float y2 = (float) (height - (DENIAL_CHAR_HEIGHT * scale * i));
			if (current_char == ' ')
				s1 = s2 = t1 = t2 = 0.0;
			x1 += xtrans;
			x2 += xtrans;
			y1 += ytrans;
			y2 += ytrans;
			if (current_char == -1)
			{
				texture_manager->change_texture("white.png");
				s1 = t1 = 0;
				s2 = t2 = 1;
			}

			//glPushMatrix();
			//glScalef(0.5, 0.5, 1.0);
			glBegin(GL_QUADS);
				glTexCoord2f(s1, t1); glVertex2f(x1, y1);
				glTexCoord2f(s1, t2); glVertex2f(x1, y2);
				glTexCoord2f(s2, t2); glVertex2f(x2, y2);
				glTexCoord2f(s2, t1); glVertex2f(x2, y1);
			glEnd();
			//glPopMatrix();
		}
	}
}

void Computer::draw_desktop()
{
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	//glDisable(GL_TEXTURE_2D); //Still curious about why this is needed but eyelids don't need it
	int width = glutGet(GLUT_WINDOW_WIDTH);
	int height = glutGet(GLUT_WINDOW_HEIGHT);
	bool pop_matrix = false;
	bool started_browser = false;
	bool switched_buffer = false;
	for (int i = 0; i < screen_elements.size(); ++i)
	{
		//TODO this cuts off things like "icon_internet_text.png"?
		if (play_minimizing_animation && screen_elements[i]->application == minimizing_application && screen_elements[i]->name.substr(0, 5) != "icon_") //Without the icon_ check it doesn't draw, that's weird
		{
			glBindFramebuffer(GL_FRAMEBUFFER, minimizing_framebuffer);
			switched_buffer = true;
			glPushMatrix();
			glTranslatef(-min_x1, -min_y1, 0.0);
			pop_matrix = true;
		}

		else
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		
		if (screen_elements[i]->application == BROWSER && !started_browser)
			started_browser = true;

		if (started_browser && screen_elements[i]->application != BROWSER && greyed_out && screen_elements[i]->name.substr(0, 5) != "icon_")
		{
			ScreenElement grey(0, 56, 1920, 1012, "greyout.png");
			//grey.draw(texture_manager);
			started_browser = false;
		}

		if (screen_elements[i]->is_visible && !screen_elements[i]->drawn_by_browser)
			screen_elements[i]->draw(texture_manager);

		if (pop_matrix)
			glPopMatrix();

		if (everything_stuck && switched_buffer)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			if (screen_elements[i]->is_visible)
				screen_elements[i]->draw(texture_manager);

			switched_buffer = false;
		}

		if (play_minimizing_animation && i != screen_elements.size() - 1 && screen_elements[i + 1]->name == "beige2.png")
		{
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			float scale;
			if (minimize_mode == 0)
				scale = minimizing_alpha + ((1.0 - minimizing_alpha) / 1.8);

			else if (minimize_mode == 1 || minimize_mode == 2)
				scale = (float)((min_x2 - min_x1) - 2 * (cur_minimize_x1 - min_x1)) / (float)(min_x2 - min_x1);

			glBindTexture(GL_TEXTURE_2D, minimizing_texture);
			glBegin(GL_QUADS);
			glColor4f(1.0, 1.0, 1.0, minimizing_alpha);
			if (minimize_mode == 0)
			{
				glTexCoord2f(0.0, 0.0); glVertex2f(cur_minimize_x1, cur_minimize_y1);
				glTexCoord2f(0.0, 1.0); glVertex2f(cur_minimize_x1, cur_minimize_y1 + (min_y2 - min_y1) * scale);
				glTexCoord2f(1.0, 1.0); glVertex2f(cur_minimize_x1 + (min_x2 - min_x1) * scale, cur_minimize_y1 + (min_y2 - min_y1) * scale);
				glTexCoord2f(1.0, 0.0); glVertex2f(cur_minimize_x1 + (min_x2 - min_x1) * scale, cur_minimize_y1);
			}

			else 
			{
				glTexCoord2f(0.0, 0.0); glVertex2f(cur_minimize_x1, cur_minimize_y1);
				glTexCoord2f(0.0, 1.0); glVertex2f(cur_minimize_x1, min_y2 - (cur_minimize_y1 - min_y1));
				glTexCoord2f(1.0, 1.0); glVertex2f(min_x2 - (cur_minimize_x1 - min_x1), min_y2 - (cur_minimize_y1 - min_y1));
				glTexCoord2f(1.0, 0.0); glVertex2f(min_x2 - (cur_minimize_x1 - min_x1), cur_minimize_y1);
			}

			glEnd();
			glColor4f(1.0, 1.0, 1.0, 1.0);
		}
	}
	
	if (draw_arrow_thing)
	{
		draw_arrow_thing = false;
		ScreenElement arrow_thing(scroll_lock_x - 20, scroll_lock_y - 20, scroll_lock_x + 20, scroll_lock_y + 20, "arrowthing.png");
		arrow_thing.draw(texture_manager);
	}

	//draw_string(texture_manager, 32, "cmmx " + to_string(cmmx) + " cmmy " + to_string(1080 - cmmy - 1), 10.0, 750.0);

	/*cmmy = 1080.0 - cmmy;
	glDisable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glColor4f(1.0, 0.0, 0.0, 1.0);
	glVertex3f(cmmx, cmmy, -0.1);
	glVertex3f(cmmx + 10.0, cmmy, -0.1);
	glVertex3f(cmmx + 10.0, cmmy + 10.0, -0.1);
	glVertex3f(cmmx, cmmy + 10.0, -0.1);
	glEnd();*/

	ScreenElement cursorelem = ScreenElement(prev_mouse_x, prev_mouse_y - texture_manager->get_height(cursor), prev_mouse_x + texture_manager->get_width(cursor), prev_mouse_y, cursor);
	if (cursor == "cursor.png")
		cursorelem.translate(-1, 1);

	else if (cursor == "cursori.png")
		cursorelem.translate(-6, 5);

	else if (cursor == "cursoriwhite.png")
		cursorelem.translate(-6, 5);

	else if (cursor == "cursorhand.png")
		cursorelem.translate(-8, 1);

	else if (cursor == "brushcursor.png")
		cursorelem.translate(-8, 9);

	else if (cursor == "cursorfill.png")
		cursorelem.translate(-22, 9);

	else if (cursor == "cursorpicker.png")
		cursorelem.translate(0, 16);

	else if (cursor == "cursormove.png")
		cursorelem.translate(-15, 15);

	else if (cursor == "cursorleftright.png")
		cursorelem.translate(-15, 15);

	cursorelem.draw(texture_manager);

	glDisable(GL_TEXTURE_2D);
	//
	glColor4f(1.0, 0.0, 0.0, 1.0);
	/*if (forcing_cursor)
	{
		glBegin(GL_LINES);
		glVertex2f(-1, first_m * -1 + first_b);
		glVertex2f(1921, first_m * 1921 + first_b);
		glVertex2f(-1, second_m * -1 + second_b);
		glVertex2f(1921, second_m * 1921 + second_b);
		glVertex2f(-1, movable_m * -1 + movable_b);
		glVertex2f(1921, movable_m * 1921 + movable_b);
	}*/
	glColor4f(1.0, 1.0, 1.0, 1.0);
	
	glEnable(GL_DEPTH_TEST); //was above texture
	glBegin(GL_QUADS);
		glColor4f(0.0, 0.0, 0.0, alpha);
		glVertex3f(0.0, 0.0, -0.1);
		glVertex3f(width, 0.0, -0.1);
		glVertex3f(width, height, -0.1);
		glVertex3f(0.0, height, -0.1);
	glEnd();
	glColor4f(1.0, 1.0, 1.0, 1.0);
}

void Computer::draw()
{
	if (computer_state == STATE_STARTUP)
		draw_messages(0, 0, 0.5);

	else if (computer_state == STATE_DESKTOP || computer_state == STATE_TITLE_SCREEN)
		draw_desktop();
}

void Computer::set_cursor(string new_cursor)
{
	if (cursor_lock)
		return;

	cursor = new_cursor;
}

void Computer::animate_messages()
{
	static int frames = 0;
	frames++;
	//old place

	if (frames % 20 == 0)
	{
		auto row = char_grid[char_grid.size() - 1];
		if (row[row.size() - 1] == -1)
			char_grid[char_grid.size() - 1].pop_back();
	}

	else if (frames % 10 == 0)
		char_grid[char_grid.size() - 1].push_back(-1);

	int current_time = glutGet(GLUT_ELAPSED_TIME);
	current_wait -= (current_time - prev_time);
	prev_time = current_time;
	if (current_wait <= 0)
	{
		if (char_grid.size() != 0)
		{
			if (char_grid[char_grid.size() - 1][char_grid[char_grid.size() - 1].size() - 1] == -1)
				char_grid[char_grid.size() - 1].pop_back();
		}

		if (messages.size() == 0)
		{
			computer_state = STATE_DESKTOP;
			reset_computer();
			cursor = "cursor.png";
			sound_manager->play_sound("startupmusic.wav");
			save_game(save_slot);
			return; //need this?
		}

		if (messages.size() == 1 || messages.size() == 34 || messages.size() == 39)
			sound_manager->play_sound("startupbeep.wav");

		Message message = messages[0];
		messages.erase(messages.begin());
		current_wait = message.delay_ms;
		vector<char> current_row;
		for (int i = 0; i < message.text.size(); ++i)
		{
			current_row.push_back(message.text[i]);
			if (current_row.size() == grid_width || i == message.text.size() - 1)
			{
				char_grid.push_back(current_row);
				current_row.clear();
				if (char_grid.size() > grid_height)
					char_grid.erase(char_grid.begin());
			}
		}
	}
}

void Computer::animate_desktop()
{
	if (freeze_time > 0)
	{
		freeze_time++;
		if (freeze_time >= 4 * 60)
		{
			start_application(INFO_BOX, "Browser has encountered a problem and needs to close.");
			started_crashing_browser = true;
			freeze_time = 0;
		}
	}

	if (title_delay > 0)
		title_delay--;
	
	if ( ((alpha > 0.0 && alpha_increment < 0) || (alpha < 1.0 && alpha_increment > 0)) && title_delay <= 0 )
		alpha += alpha_increment;

	if (alpha == 1.0 && title_delay <= 0)
	{
		if (!everything_stuck)
			computer_done = true;

		else
		{
			going_to_sleep = false;
			alpha_increment = -ALPHA_INCREMENT;
		}
	}

	if (queue_click_x != -1 && queue_click_y != -1)
	{
		mouse_clicked(GLUT_LEFT_BUTTON, GLUT_DOWN, queue_click_x, queue_click_y);
		queue_click_x = -1;
		queue_click_y = -1;
	}
}

void Computer::animate()
{
	if (flash_counter != -1)
	{
		if (flash_counter % 3 == 0)
		{
			if (focus_application == NONE)
				focus_application = INFO_BOX;

			else focus_application = NONE;
			update_focus();
		}

		flash_counter++;
		if (flash_counter > 30)
		{
			focus_application = INFO_BOX;
			update_focus();
			flash_counter = -1;
		}
	}
	
	if (animate_fake)
		move_fake();

	if (play_minimizing_animation)
	{
		if (minimize_mode == 0)
		{
			if (alpha_going_up)
			{
				cur_minimize_x1 -= (target_minimize_x - min_x1) * 0.75 / 10.0;
				cur_minimize_y1 -= (0 - min_y1) * 0.75 / 10.0;
				minimizing_alpha += 0.1;
				if (minimizing_alpha >= 1.0)
					end_minimizing_animation();
			}

			else
			{
				cur_minimize_x1 += (target_minimize_x - min_x1) * 0.75 / 10.0;
				cur_minimize_y1 += (0 - min_y1) * 0.75 / 10.0;
				minimizing_alpha -= 0.1;
				if (minimizing_alpha <= 0.0)
					end_minimizing_animation();
			}
		}

		else if (minimize_mode == 1)
		{
			minimizing_alpha += 0.1;
			cur_minimize_x1--;
			cur_minimize_y1--;
			if (minimizing_alpha >= 1.0)
				end_minimizing_animation();
		}

		else if (minimize_mode == 2)
		{
			minimizing_alpha -= 0.1;
			cur_minimize_x1++;
			cur_minimize_y1++;
			if (minimizing_alpha <= 0.0)
				end_minimizing_animation();
		}
	}
	
	if (computer_state == STATE_STARTUP)
		animate_messages();

	else if (computer_state == STATE_DESKTOP || computer_state == STATE_TITLE_SCREEN)
	{
		animate_desktop();
		for (int i = 0; i < screen_elements.size(); ++i)
			screen_elements[i]->animate();

		update_focus();
	}

	process_control_changes();
}

void Computer::end_minimizing_animation()
{
	play_minimizing_animation = false;
	glDeleteTextures(1, &minimizing_texture);
	glDeleteFramebuffers(1, &minimizing_framebuffer);
	glDeleteRenderbuffers(1, &minimizing_rbo);
}

void Computer::go_to_desktop()
{
	//TODO Do you want to actually call close_application() on everything?
	computer_state = STATE_DESKTOP;
	applications_open = 0;
	decrypt_forcing = false;
	screen_elements.clear();
	screen_elements.push_back(make_unique<BackgroundAnimation>(0.0, 0.0, 1920.0, 1080.0, "background.png"));
	screen_elements.push_back(make_unique<DesktopIcon>(50.0, 900, 100.0, 956, "icon_internet_text.png", this, BROWSER)); //- 50 * 1.117
	screen_elements.push_back(make_unique<DesktopIcon>(50.0, 800, 100.0, 856, "icon_folder_text.png", this, FILES));
	screen_elements.push_back(make_unique<DesktopIcon>(50.0, 700, 100.0, 756, "icon_console_text.png", this, CONSOLE));
	//screen_elements.push_back(make_unique<DesktopIcon>(50.0, 600.0, 100.0, 650.0, "icon_paint.png", this, PAINT));
	for (int i = 0; i < extra_apps.size(); ++i)
		screen_elements.push_back(make_unique<DesktopIcon>(50.0, (6 - i)*100.0, 50 + texture_manager->get_width(extra_apps[i].first), (6 - i)*100.0 + texture_manager->get_height(extra_apps[i].first), extra_apps[i].first, this, extra_apps[i].second));

	screen_elements.push_back(make_unique<ScreenElement>(0.0, 0.0, 1920.0, 56.0, "beige2.png"));
	screen_elements.push_back(make_unique<PowerSymbol>(0.0, 0.0, 69.0, 52.0, "powersymbol1.png", this));
	focus_element = screen_elements[0].get();
	glutWarpPointer(prev_mouse_x, 1080 - prev_mouse_y);
}

int Computer::get_internet_icon()
{
	for (int i = 0; i < screen_elements.size(); ++i)
	{
		if (screen_elements[i]->name == "icon_internet_text.png")
			return i;
	}

	return 0;
}

void Computer::delete_save_file(int file_number)
{
	string name = "savedata" + to_string(file_number);
	remove(name.c_str());
	string folder_name = name + "p";
	char* buffer = (char*) calloc(33000, sizeof(char));
	GetFullPathName(folder_name.c_str(), 33000, buffer, NULL);
	SHFILEOPSTRUCT file_op = { NULL, FO_DELETE, buffer, "", FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT, false, 0, "" };
	int ret = SHFileOperation(&file_op);
	free(buffer);
}

void Computer::go_to_continue_screen()
{
	for (int i = 0; i < screen_elements.size(); ++i)
		screen_elements[i]->marked_for_deletion = true;

	to_be_added.push_back(make_unique<ScreenElement>(0, 0, 1920, 1080, "title2.png"));
	int cur_x = 128;
	int cur_y = 1080 - 204;
	for (int i = 1; i <= 10; ++i)
	{
		ifstream in("savedata" + to_string(i), ios::binary);
		if (in.good())
		{
			to_be_added.push_back(make_unique<Button>(cur_x, cur_y, -1, -1, "filenormal.png", NONE, [this, i]()
			{
				load_game("savedata" + to_string(i));
				this->save_slot = "savedata" + to_string(i);
				this->go_to_sleep();
			}, []() {}, "", "filelight.png", "filenormal.png"));
			string tm = read_binary<string>(in);
			to_be_added.push_back(make_unique<InfoText>(cur_x + 132, cur_y + 85, cur_x + 132, cur_y + 85, "invisible.png", tm, NONE, false));
			int elapsed_time = read_binary<int>(in);
			int total_minutes = (elapsed_time / 1000) / 60;
			int hours = total_minutes / 60;
			int minutes = total_minutes % 60;
			string minutes_string = to_string(minutes);
			if (minutes_string.length() == 1)
				minutes_string = "0" + minutes_string;

			to_be_added.push_back(make_unique<InfoText>(cur_x + 132, cur_y + 38, cur_x + 132, cur_y + 38, "invisible.png", to_string(hours) + ":" + minutes_string, NONE, false));
			to_be_added.push_back(make_unique<Button>(870 + (cur_x - 128), 1080 - 117 + (cur_y - (1080 - 204)), -1, -1, "delete1.png", NONE, [this, i, cur_x, cur_y]()
			{
				auto delete_menu = make_unique<ScreenElement>(792 + (cur_x - 128), 1080 - 193 + (cur_y - (1080 - 204)), -1, -1, "deletemenu.png", NONE);
				ScreenElement* delete_menu_ptr = delete_menu.get();
				to_be_added.push_back(std::move(delete_menu));
				auto yes_button = make_unique<Button>(842 - 7 + (cur_x - 128), 1080 - 141 + (cur_y - (1080 - 204)), -1, -1, "yes1.png", NONE, [this, i]()
				{
					delete_save_file(i);
					go_to_continue_screen();
				}, []() {}, "", "yes2.png", "yes2.png");
				ScreenElement* yes_button_ptr = yes_button.get();
				to_be_added.push_back(std::move(yes_button));
				auto no_button = make_unique<Button>(845 - 8 + (cur_x - 128), 1080 - 172 + (cur_y - (1080 - 204)), -1, -1, "no1.png", NONE, []() {}, []() {}, "", "no2.png", "no2.png");
				ScreenElement* no_button_ptr = no_button.get();
				no_button->release_function = [no_button_ptr, yes_button_ptr, delete_menu_ptr]()
				{
					no_button_ptr->marked_for_deletion = true;
					yes_button_ptr->marked_for_deletion = true;
					delete_menu_ptr->marked_for_deletion = true;
				};
				to_be_added.push_back(std::move(no_button));
			}, []() {}, "", "delete2.png", "delete2.png"));
		}

		else to_be_added.push_back(make_unique<ScreenElement>(cur_x, cur_y, -1, -1, "filedark.png", NONE));
		in.close();
		cur_y -= 183;
		if (i == 5)
		{
			cur_x = 994;
			cur_y = 1080 - 204;
		}
	}

	to_be_added.push_back(make_unique<Button>(48, 109, -1, -1, "return1.png", NONE, [this]()
	{
		this->go_to_title_screen();
	}, []() {}, "", "return2.png", "return2.png"));
	this->focus_element = to_be_added[0].get();
}

void Computer::go_to_title_screen()
{
	applications_open = 0;
	for (int i = 0; i < screen_elements.size(); ++i)
		screen_elements[i]->marked_for_deletion = true;

	to_be_added.push_back(make_unique<ScreenElement>(0, 0, 1920, 1080, "title2.png"));
	to_be_added.push_back(make_unique<ScreenElement>(48, 109, -1, -1, "title.png"));
	to_be_added.push_back(make_unique<Button>(48, 109 - 21, -1, -1, "newgame1.png", NONE, [this]()
	{
		int i = 1;
		for (i = 1; i <= 10; ++i)
		{
			ifstream in("savedata" + to_string(i), ios::binary);
			if (!in.good())
				break;
		}

		this->save_slot = "savedata" + to_string(i);
		this->go_to_sleep();
	}, []() {}, "", "newgame2.png", "newgame2.png"));
	to_be_added.push_back(make_unique<Button>(48, 109 - 21 - 21, -1, -1, "continue1.png", NONE, [this]() 
	{
		go_to_continue_screen();
	}, []() {}, "", "continue2.png", "continue2.png"));
	focus_element = to_be_added[0].get();
	if (!been_to_title_screen)
	{
		glutWarpPointer(100, 100);
		prev_mouse_x = 100;
		prev_mouse_y = 100;
		been_to_title_screen = true;
	}
}

void Computer::go_to_sleep()
{
	going_to_sleep = true;
	alpha_increment = ALPHA_INCREMENT;
	for (int i = 0; i < screen_elements.size(); ++i)
		screen_elements[i]->mouse_off();

	stop_website_sounds();
}

void Computer::reset_computer()
{
	computer_done = false;
	going_to_sleep = false;
	alpha_increment = -ALPHA_INCREMENT;
	if (computer_state == STATE_DESKTOP)
		go_to_desktop();

	else if (computer_state == STATE_TITLE_SCREEN)
	{
		go_to_title_screen();
		process_control_changes();
	}
}

//TODO maybe upclick on different element than focus element goes focusless and does not focus on "clicked" element
void Computer::mouse_clicked(int button, int state, int x, int y)
{
	clicked_element = NULL;
	bool pass_to_browser = false;
	if (going_to_sleep)
		return;

	if (title_delay > 0)
		return;

	bool have_info_box = false;
	for (int i = 0; i < screen_elements.size(); ++i)
	{
		if (screen_elements[i].get()->application == INFO_BOX)
			have_info_box = true;
	}
	
	if (read_email["rosecolored3bold.png"] && !started_crashing_browser && queue_click_x == -1 && queue_click_y == -1 && button == GLUT_LEFT && state == GLUT_DOWN && freeze_time == 0)
		freeze_time = 1;

	y = glutGet(GLUT_WINDOW_HEIGHT) - y; //-1?
	x = (int)((float)x * (1920.0 / (float)glutGet(GLUT_WINDOW_WIDTH)));
	y = (int)((float)y * (1080.0 / (float)glutGet(GLUT_WINDOW_HEIGHT)));
	bool already_clicked = false;
	Canvas* canvas = dynamic_cast<Canvas*>(focus_element);
	if (canvas != NULL && button != 0 && canvas->mouse_held)
	{
		focus_element->mouse_clicked(button, state, x, y);
		return;
	}

	if (have_active_fake && active_fake.point_in_rect_strict(x, y))
		animate_fake = true;
	
	for (int i = screen_elements.size() - 1; i >= 0; --i)
	{
		if (freeze_time > 0)
			continue;
		
		ScreenElement* se = screen_elements[i].get();
		bool did_scale = false;
		if (se->no_scale)
		{
			x = scalex(x);
			y = scaley(y);
			did_scale = true;
		}

		if (!se->is_visible)
			continue;

		if (state == GLUT_DOWN)
		{
			if ( (se->x1 <= x && x <= se->x2 && se->y1 <= y && y <= se->y2 || se->rotated && se->check_rotation(x, y)) && !already_clicked && !se->never_focus)
			{
				if (have_info_box && (se->application != INFO_BOX || se->info_box_id != info_box_id))
				{
					flash_counter = 0;
					continue;
				}

				bool pass_to_browser = se->browser_child && (button == 1 || button == 3 || button == 4);
				if (se->responds_to(button) && !pass_to_browser)
				{
					se->give_focus(); //TODO first two lines used to be reversed
					focus_application = se->application;
					se->mouse_clicked(button, state, x, y);
					clicked_element = se;
					focus_element = se;
				}

				if (!pass_to_browser)
					already_clicked = true;
			}

			else
				se->take_focus();
		}

		else if (state == GLUT_UP)
		{
			if (se->has_focus)
			{
				if (have_info_box && (se->application != INFO_BOX || se->info_box_id != info_box_id) && !already_clicked)
				{
					//flash_counter = 0; TODO I don't think there's a need to flash here
					continue;
				}
				
				if (se->x1 <= x && x <= se->x2 && se->y1 <= y && y <= se->y2 && !already_clicked)
				{
					se->mouse_clicked(button, state, x, y);
					already_clicked = true;
				}

				else
				{
					if (se->receive_out_of_bounds_clicks)
						se->mouse_clicked(button, state, x, y);

					if (se->surrender_focus_on_up_click) //TODO this exists so e.g. scroll bar will release when upclick outside of it.  For console, just write real focus switching function
					{
						se->take_focus();
						focus_element = screen_elements[0].get();
					}
				}
			}
		}

		if (did_scale)
		{
			x = unscalex(x);
			y = unscaley(y);
		}
	}

	process_control_changes();
	update_focus();
}

void Computer::give_focus_to(ScreenElement* se)
{
	se->give_focus();
	focus_application = se->application;
	focus_element = se;
}

void Computer::update_focus()
{
	for (int i = 0; i < screen_elements.size(); ++i)
	{
		if (screen_elements[i]->border_element)
		{
			if (screen_elements[i]->application == focus_application &&
				(focus_application == INFO_BOX && screen_elements[i]->info_box_id == info_box_id ||
					focus_application != INFO_BOX))
			{
				if (screen_elements[i]->name[0] == 'q')
					screen_elements[i]->name = screen_elements[i]->name.substr(1, screen_elements[i]->name.size());
			}

			else
			{
				if (screen_elements[i]->name[0] != 'q')
					screen_elements[i]->name.insert(0, 1, 'q');
			}
		}
	}

	if (focus_application != NONE && focus_application != INFO_BOX && !(focus_element->name.size() >= 9 && focus_element->name.substr(focus_element->name.size() - 8, 4) == "tray"))
		foreground_if_obscured(focus_application, "");
}

void Computer::process_control_changes()
{
	for (int i = 0; i < screen_elements.size(); ++i)
	{
		if (screen_elements[i]->transient)
		{
			screen_elements[i]->marked_for_deletion = true;
			for (int j = 0; j < screen_elements.size(); ++j)
			{
				if (screen_elements[j]->name == "altcapbuttonhover.png")
					screen_elements[j]->name = "altcapbutton.png";
			}
		}
	}
	
	remove_deleted();
	for (int i = 0; i < to_be_added.size(); ++i)
	{
		if (to_be_added[i]->name.substr(0, 5) == "icon_")
			screen_elements.push_back(std::move(to_be_added[i]));

		else if (to_be_added[i]->browser_child)
		{
			int j = 0;
			for (j = 0; j < screen_elements.size(); ++j)
			{
				if (dynamic_cast<ScrollPanel*>(screen_elements[j].get()))
					break;
			}

			screen_elements.insert(screen_elements.begin() + j + 1, std::move(to_be_added[i]));
		}

		else
		{
			int end = -1;
			for (int j = 0; j < screen_elements.size(); ++j)
			{
				if (screen_elements[j]->application == to_be_added[i]->application && screen_elements[j]->name.substr(0, 5) != "icon_")
					end = j;
			}

			if (end != -1)
				screen_elements.insert(screen_elements.begin() + end + 1, std::move(to_be_added[i]));

			else
			{
				int offset = computer_state == STATE_DESKTOP ? 2 : 0;
				screen_elements.insert(screen_elements.end() - offset - applications_open, std::move(to_be_added[i]));
			}
		}
	}

	to_be_added.clear();
}

void Computer::start_forcing_cursor(float fm, float fb, float sm, float sb, float mm, float mb, float em, float eb, bool up)
{
	first_m = fm;
	first_b = fb;
	second_m = sm;
	second_b = sb;
	movable_m = mm;
	movable_b = mb;
	end_m = em;
	end_b = eb;
	going_up = up;
	forcing_cursor = true;
}

void Computer::force_to(int x, int y)
{
	forcing_to_x = x;
	forcing_to_y = y;
	/*float m = (float)(prev_mouse_y - y) / (float)(prev_mouse_x - x);
	float b = y - m*x;
	start_forcing_cursor(m, b - 100, m, b + 100, -1.0 / m, y > prev_mouse_y ? prev_mouse_y - (-1.0 / m)*prev_mouse_x - 5 : prev_mouse_y - (-1.0 / m)*prev_mouse_x, -1.0 / m, y - (-1.0 / m)*x, y > prev_mouse_y);*/
	forcing_bound_x1 = min(forcing_to_x, prev_mouse_x);
	forcing_bound_x2 = max(forcing_to_x, prev_mouse_x);
	forcing_bound_y1 = min(forcing_to_y, prev_mouse_y);
	forcing_bound_y2 = max(forcing_to_y, prev_mouse_y);
	current_forcing_x = prev_mouse_x;
	current_forcing_y = prev_mouse_y;
	forcing_cursor = true;
}

//TODO if you make a beeling for the Decrypt button before the first forcing, it will be in your forcing track and you will be able to click it on your way up
void Computer::warp_cursor(int x, int y)
{
	/*int warp_x = ceil((float)(movable_b - first_b) / (float)(first_m - movable_m)) + (first_m < 0 ? 20 : -20);
	int warp_y = glutGet(GLUT_WINDOW_HEIGHT) - (movable_m * warp_x + movable_b) + (going_up ? -5 : 5);
	bool warp = false;
	int lower_y = first_m * x + first_b;
	if (y < lower_y)
		warp = true;

	int upper_y = second_m * x + second_b;
	if (y > upper_y)
		warp = true;

	int movable_y = movable_m * x + movable_b;
	int end_y = end_m * x + end_b;
	bool end_of_forcing = false;
	if (going_up)
	{
		if (y < movable_y)
			warp = true;

		if (y > end_y)
			warp = end_of_forcing = true;
	}

	else
	{
		if (y > movable_y)
			warp = true;

		if (y < end_y)
			warp = end_of_forcing = true;
	}

	if (warp)
	{
		if (end_of_forcing)
		{
			end_of_forcing_function();
		}

		glutWarpPointer(warp_x, warp_y);
		return;
	}

	if (abs(end_y - y) > 30 && abs(y - movable_y) >= 7)
		movable_b += (y - movable_y) - copysign(2, y - movable_y);*/

	if (forcing_bound_x1 <= x && x <= forcing_bound_x2 && forcing_bound_y1 <= y && y <= forcing_bound_y2)
	{
		forcing_bound_x1 = min(forcing_to_x, x);
		forcing_bound_x2 = max(forcing_to_x, x);
		forcing_bound_y1 = min(forcing_to_y, y);
		forcing_bound_y2 = max(forcing_to_y, y);
		current_forcing_x = x;
		current_forcing_y = y;
	}

	else if (sqrt(pow(x - current_forcing_x, 2) + pow(y - current_forcing_y, 2)) > 25)
		glutWarpPointer(scalex(current_forcing_x), scaley(1080 - current_forcing_y));

	if (abs((forcing_bound_x1 - forcing_bound_x2) * (forcing_bound_y1 - forcing_bound_y2)) < 16)
		end_of_forcing_function();
}

void Computer::mouse_moved(int x, int y, ScreenElement* do_not_inform, bool no_scale)
{
	if (no_mouse)
		return;

	int unscale_x = 0;
	int unscale_y = 0;
	cmmx = x;
	cmmy = y;
	if (no_scale)
		y = 1080 - y;

	else y = glutGet(GLUT_WINDOW_HEIGHT) - y; //-1?
	if (!no_scale)
	{
		x = (int)((float)x * (1920.0 / (float)glutGet(GLUT_WINDOW_WIDTH)));
		y = (int)((float)y * (1080.0 / (float)glutGet(GLUT_WINDOW_HEIGHT)));
	}

	if (sticking_cursor && x == sticking_x && y == sticking_y)
		return;

	if (forcing_cursor)
		warp_cursor(x, y);

	if (sticking_cursor)
		glutWarpPointer(scalex(sticking_x), scaley(1080 - sticking_y));

	if (looking_for_stick)
	{
		if (y < 400)
		{
			sticking_cursor = true;
			sticking_x = x;
			sticking_y = y;
			looking_for_stick = false;
		}
	}

	if (forcing_e2)
	{
		if (y < 56)
			glutWarpPointer(x, 200);

		else if (y > 1044)
			glutWarpPointer(x, 900);
	}

	bool already_moused_over = false;
	someone_set_cursor = false;
	bool have_info_box = false;
	for (int i = 0; i < screen_elements.size(); ++i)
	{
		if (screen_elements[i].get()->application == INFO_BOX)
			have_info_box = true;
	}

	for (int i = screen_elements.size() - 1; i >= 0; --i)
	{
		if (freeze_time > 0)
			continue;

		ScreenElement* se = screen_elements[i].get();
		if (!se->is_visible)
			continue;

		if (have_info_box && (se->application != INFO_BOX || se->info_box_id != info_box_id))
			continue;

		bool did_scale = false;
		if (se->no_scale)
		{
			unscale_x = x;
			unscale_y = y;
			x = scalex(x);
			y = scaley(y);
			did_scale = true;
		}

		bool was_over = false;
		if (se->x1 <= x && x <= se->x2 && se->y1 <= y && y <= se->y2 && !already_moused_over && !se->never_focus)
		{
			if (!going_to_sleep && se != do_not_inform)
				se->mouse_over(x, y);

			already_moused_over = true;
			was_over = true;
		}

		else se->mouse_off();
        
        if ((se->has_focus && se != do_not_inform) || (was_over && se->always_inform_mouse_moved))
            se->mouse_moved(x, y);

		if (did_scale)
		{
			x = unscale_x;
			y = unscale_y;
		}
	}

	if (!someone_set_cursor && !maintain_cursor && computer_state == STATE_DESKTOP)
	{
		cursor = "cursor.png";
	}

	prev_mouse_x = x;
	prev_mouse_y = y;
}

void Computer::borders_title_x(int ex1, int ey1, int ex2, int ey2, Application application, bool do_x, function<void()> on_close, bool no_move)
{
	if (no_move)
	{
		auto top = make_unique<ScreenElement>(ex1 - 2, ey2, ex2 + 2, ey2 + 30, "silver.png", application, false, true); //top
		top->partial_x = true;
		to_be_added.push_back(std::move(top));
	}

	else
	{
		auto top = make_unique<DragElement>(ex1 - 2, ey2, ex2 + 2, ey2 + 30, "silver.png", this, application, false, true); //top
		top->partial_x = true;
		to_be_added.push_back(std::move(top));
	}

	to_be_added.push_back(make_unique<ScreenElement>(ex1 - 4, ey1, ex1, ey2 + 30, "windowborder1.png", application));
	to_be_added.push_back(make_unique<ScreenElement>(ex2, ey1, ex2 + 4, ey2 + 30, "windowborder1.png", application));
	to_be_added.push_back(make_unique<ScreenElement>(ex1, ey1 - 4, ex2, ey1, "windowborder2.png", application));
	to_be_added.push_back(make_unique<ScreenElement>(ex1, ey2 + 30, ex2, ey2 + 34, "windowborder2.png", application));
	to_be_added.push_back(make_unique<ScreenElement>(ex1 - 4, ey1 - 4, ex1, ey1, "corner1.png", application));
	to_be_added.push_back(make_unique<ScreenElement>(ex1 - 4, ey2 + 30, ex1, ey2 + 34, "corner2.png", application));
	to_be_added.push_back(make_unique<ScreenElement>(ex2, ey2 + 30, ex2 + 4, ey2 + 34, "corner3.png", application));
	to_be_added.push_back(make_unique<ScreenElement>(ex2, ey1 - 4, ex2 + 4, ey1, "corner4.png", application));
	if (do_x)
		to_be_added.push_back(make_unique<Button>(ex2 - 81, ey2, ex2, ey2 + 30, "xbutton.png", application, [this, application, on_close]() {on_close(); this->close_application(application);}, []() {}, "", "xbuttonlight.png", "xbuttondark.png", "", false, true));
}

bool Computer::application_open(Application application)
{
	for (int i = 0; i < screen_elements.size(); ++i)
	{
		if (screen_elements[i]->application == application)
			return true;
	}

	return false;
}

void Computer::start_application(Application application, string info_text, bool file_dialog, bool progress_dialog, int download_action, ScrollPanel* scroll_panel, bool no_move)
{
	if (forcing_cursor)
		exit(0);
	
	if (decrypt_forcing && application != INFO_BOX && application != DECRYPTION_APP)
	{
		start_application(INFO_BOX, "System is using too much memory.");
		return;
	}

	if (application != INFO_BOX && application_open(application) && !(application == FILES && file_dialog))
	{
		auto minimize = minimize_function(application, "", false);
		minimize();
		if (screen_elements[0]->application == application)
			minimize();

		return;
	}
	
	if (application != INFO_BOX)
		applications_open++;

	focus_application = application;
	reset_elements();
	switch (application)
	{
	case BROWSER:
	{
		if (showed_walter && !reached_space_game)
			required_target = "walter3";

		int toolbar_base = 1080 - WINDOW_TITLE_HEIGHT - BROWSER_TOOLBAR_HEIGHT;
		int window_title_base = 1080 - WINDOW_TITLE_HEIGHT;
		if (crashed_browser && !recovered_browser)
		{
			start_application(INFO_BOX, "Browser is already running.  Close existing instance.");
			applications_open--;
			return;
		}
		
		auto minimize = minimize_function(BROWSER, "icon_internet_tray.png");
		auto tray = tray_function(BROWSER, "icon_internet_tray.png");
		to_be_added.push_back(make_unique<Button>(65.0 * applications_open + 15, 0.0, (65.0 * applications_open + 15) + 60.0, 50.0, "icon_internet_tray.png", BROWSER, [this, tray]()
		{
			tray();
			if (this->everything_stuck)
				this->tray_bucket++;
		}, []() {}, "", "sheen.png", "sheen2.png"));
		unique_ptr<ScrollPanel> scroll_panel = make_unique<ScrollPanel>(0.0, 0.0, 1920.0, 1080.0, this, BROWSER);
		to_be_added.insert(to_be_added.end() - 7, std::move(scroll_panel));
		ScrollPanel* scroll_panel_pointer = reinterpret_cast<ScrollPanel*>(to_be_added[to_be_added.size() - 8].get());
		to_be_added.push_back(make_unique<ScreenElement>(1900, 56, 1920, toolbar_base, "scrollbackground.png", BROWSER));
		unique_ptr<ScrollBar> scroll_bar = make_unique<ScrollBar>(1901.0, 0.0, 1920.0, toolbar_base - 20, "scrollbar.png", BROWSER, scroll_panel_pointer);
		to_be_added.push_back(std::move(scroll_bar));
		ScrollBar* scroll_bar_pointer = reinterpret_cast<ScrollBar*>(to_be_added[to_be_added.size() - 1].get());
		scroll_panel_pointer->scroll_bar = scroll_bar_pointer;
		scroll_bar_pointer->max_y = toolbar_base - 20;
		to_be_added.push_back(make_unique<ScreenElement>(0.0, window_title_base, 1920.0, 1080.0, "silver.png", BROWSER, false, true));
		to_be_added.push_back(make_unique<ScreenElement>(10.0, window_title_base + 5, 30.0, 1080 - 5, "|icon_internet.png", BROWSER));
		to_be_added.push_back(make_unique<ScreenElement>(35, window_title_base + 3, 35 + 125, 1080 - 2, "internet_name.png", BROWSER));
		/*to_be_added.push_back(make_unique<Button>(1771 + 32, toolbar_base + 9, 1875 + 32, toolbar_base + 29, "invisible.png", BROWSER,
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->start_loading_site("useful", false);
		}, [](){}, "", "bookmarkcover.png", "bookmarkcoverdark.png", "", true));
		to_be_added.push_back(make_unique<Button>(1647 + 32, toolbar_base + 9, 1745 + 32, toolbar_base + 29, "invisible.png", BROWSER,
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->start_loading_site("nenda", false);
		}, [](){}, "", "bookmarkcover.png", "bookmarkcoverdark.png", "", true));
		to_be_added.push_back(make_unique<Button>(1522 + 32, toolbar_base + 9, 1622 + 32, toolbar_base + 29, "invisible.png", BROWSER,
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->start_loading_site("reddit", false);
		}, [](){}, "", "bookmarkcover.png", "bookmarkcoverdark.png", "", true));
		to_be_added.push_back(make_unique<Button>(1456 + 32, toolbar_base + 9, 1497 + 32, toolbar_base + 29, "invisible.png", BROWSER,
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->start_loading_site("ain", false);
		}, []() {}, "", "bookmarkcover.png", "bookmarkcoverdark.png", "", true));
		to_be_added.push_back(make_unique<Button>(1368 + 32, toolbar_base + 9, 1431 + 32, toolbar_base + 29, "invisible.png", BROWSER,
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->start_loading_site("search", false);
		}, []() {}, "", "bookmarkcover.png", "bookmarkcoverdark.png", "", true));

		to_be_added.push_back(make_unique<ScreenElement>(1771 + 32, toolbar_base + 9, 1875 + 32, toolbar_base + 29, "usefulbookmark.png", BROWSER));
		to_be_added.push_back(make_unique<ScreenElement>(1647 + 32, toolbar_base + 9, 1745 + 32, toolbar_base + 29, "nendabookmark.png", BROWSER));
		to_be_added.push_back(make_unique<ScreenElement>(1522 + 32, toolbar_base + 9, 1622 + 32, toolbar_base + 29, "socnewsbookmark.png", BROWSER));
		to_be_added.push_back(make_unique<ScreenElement>(1456 + 32, toolbar_base + 9, 1497 + 32, toolbar_base + 29, "ainbutton.png", BROWSER));
		to_be_added.push_back(make_unique<ScreenElement>(1368 + 32, toolbar_base + 9, 1431 + 32, toolbar_base + 29, "searchbookmark.png", BROWSER));
		to_be_added[to_be_added.size() - 1]->never_focus = true;
		to_be_added[to_be_added.size() - 2]->never_focus = true;
		to_be_added[to_be_added.size() - 3]->never_focus = true;
		to_be_added[to_be_added.size() - 4]->never_focus = true;
		to_be_added[to_be_added.size() - 5]->never_focus = true;*/

		//to_be_added.push_back(make_unique<ScreenElement>(1275 + 32, toolbar_base + 9, 1343 + 32, toolbar_base + 29, "bookmarks.png", BROWSER));
		to_be_added.push_back(make_unique<Button>(1920 - 81, window_title_base, 1920, 1078.0 + 2, "xbutton.png", BROWSER, [this]()
		{
			this->close_application(BROWSER);
			if (everything_stuck &&
				minimize_bucket >= 3 &&
				close_bucket >= 3 &&
				url_bucket >= 3 &&
				scroll_bucket >= 3 &&
				power_bucket >= 3 &&
				tray_bucket >= 3 &&
				breaking_stage == 0)
				increment_breaking_stage();
		}, []() {}, "", "xbuttonlight.png", "xbuttondark.png", "", false, true));
		to_be_added.push_back(make_unique<Button>(1920 - 81 - 18 - 98, window_title_base, 1920 - 81 - 18, 1078.0 + 2, "minimize.png", BROWSER, [this, minimize]()
		{
			minimize();
			if (this->everything_stuck)
				this->minimize_bucket++;
		}, []() {}, "", "minimizelight.png", "minimizedark.png", "", false, true));
		to_be_added.push_back(make_unique<Button>(1900.0, toolbar_base - 27, 1920.0, toolbar_base, "arrow.png", BROWSER,
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->release_key(KEY_UP);
		},
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->press_key(KEY_UP);
		}, "", "arrowlight.png", "arrowdark.png"));
		to_be_added.push_back(make_unique<Button>(1900.0, 56.0, 1920.0, 76.0 + 7, "arrow2.png", BROWSER,
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->release_key(KEY_DOWN);
		},
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->press_key(KEY_DOWN);
		}, "", "arrow2light.png", "arrow2dark.png"));
		//to_be_added.push_back(make_unique<ScreenElement>(7, toolbar_base + 1, 45, window_title_base - 4, "holder.png", BROWSER));
		//to_be_added.push_back(make_unique<ScreenElement>(9, toolbar_base + 1, 81, toolbar_base + 37, "navholder.png", BROWSER));
		to_be_added.push_back(make_unique<Button>(1681, toolbar_base + 9, 1722, toolbar_base + 29, "back.png", BROWSER,
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->go_back();
		},
			[](){}, "", "backlight.png", "backdark.png", "", false, true));
		//to_be_added.push_back(make_unique<ScreenElement>(83, toolbar_base + 1, 45, window_title_base - 4, "holder.png", BROWSER));
		to_be_added.push_back(make_unique<Button>(1722, toolbar_base + 9, 1785, toolbar_base + 29, "forward.png", BROWSER,
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->go_forward();
		},
			[](){}, "", "forwardlight.png", "forwarddark.png", "", false, true));

		to_be_added.push_back(make_unique<Button>(1785, toolbar_base + 9, 1833, toolbar_base + 29, "home.png", BROWSER,
			[scroll_panel_pointer]()
		{
			scroll_panel_pointer->go_home();
		},
			[]() {}, "", "homelight.png", "homedark.png", "", false, true));

		auto bookmarksbutton = make_unique<Button>(1833, toolbar_base + 9, 1833 + 78, toolbar_base + 29, "bookmarksbutton.png", BROWSER,
			[scroll_panel_pointer]()
		{

		},
			[]() {}, "", "bookmarksbuttonlight.png", "bookmarksbuttondark.png", "", false, true);
		bookmarksbutton->dropdown_button = true;
		bookmarksbutton->scroll_parent = scroll_panel_pointer;
		bookmarksbutton->parent = this;
		to_be_added.push_back(std::move(bookmarksbutton));

		//to_be_added.push_back(make_unique<Video>(300.0, 300.0, 800.0, 800.0, "blue.png", BROWSER));
		//to_be_added.push_back(make_unique<BubbleGame>(0.0, 56.0, 1920.0, 1000.0, "black.png", this, BROWSER));
		scroll_panel_pointer->change_website("search");
		break;
	}

	case FILES:
	{
		if (update_in_progress)
		{
			start_application(INFO_BOX, "Cannot use system services while system update is in progress.");
			focus_application = INFO_BOX;
			applications_open--;
			return;
		}

		Application target;
		if (file_dialog)
		{
			focus_application = target = INFO_BOX;
			applications_open--;
		}

		else
			target = FILES;
		
		auto minimize = minimize_function(FILES, "icon_folder_tray.png");
		auto tray = tray_function(FILES, "icon_folder_tray.png");
		if (!file_dialog)
			to_be_added.push_back(make_unique<Button>(65.0 * applications_open + 15, 0.0, (65.0 * applications_open + 15) + 60, 50.0, "icon_folder_tray.png", FILES, tray, [](){}, "", "sheen.png", "sheen2.png"));

		borders_title_x(770.0 - 300, file_dialog ? 394 - 120 + 8 : 404.0 - 60, 1150.0, 744.0, target);
		//to_be_added.push_back(make_unique<ScreenElement>(760.0 - 300, 744.0, 1160.0, 780.0, "silver.png", target)); //title bar
		to_be_added.push_back(make_unique<ScreenElement>(770.0 - 300 + 10, 744 + 5, 770 - 300 + 30, 744 + 25, "|icon_folder.png", target));
		to_be_added.push_back(make_unique<ScreenElement>(760 - 300 + 45, 747, 760 - 300 + 45 + 125, 747 + 25, "files_name.png", target));
		//to_be_added.push_back(make_unique<XButton>(1122.0, 747.0, 1153.0, 778.0, "xbutton.png", this, target));
		//to_be_added.push_back(make_unique<Button>(1122.0, 747.0, 1153.0, 778.0, "xbutton.png", target, [this, target]() {this->close_application(target);}, []() {}, "", "xbuttonlight.png", "xbuttondark.png"));
		if (!file_dialog)
			to_be_added.push_back(make_unique<Button>(1150 - 81 - 18 - 98, 744, 1150 - 81 - 18, 744 + 30, "minimize.png", FILES, minimize, [](){}, "", "minimizelight.png", "minimizedark.png"));

		//to_be_added.push_back(make_unique<ScreenElement>(760.0 - 300, file_dialog ? 394 - 120 : 394.0 - 60, 761 - 300, 744.0, "silver.png", target)); //left
		//to_be_added.push_back(make_unique<ScreenElement>(1159, file_dialog ? 394 - 120 : 394.0 - 60, 1160.0, 744.0, "silver.png", target)); //right
		//to_be_added.push_back(make_unique<ScreenElement>(760.0 - 300, file_dialog ? 394 - 120 : 394.0 - 60, 1160.0, file_dialog ? 744 - 60 : 744.0, "silver.png", target)); //bottom
		to_be_added.push_back(make_unique<ScreenElement>(770.0 - 300, 744 - 60, 1150, 744, "white.png", target));
		to_be_added.push_back(make_unique<ScreenElement>(770.0 - 300, 744 - 38, 1150, 744/* - 59*/, "beige.png", target));
		if (file_dialog)
			to_be_added.push_back(make_unique<ScreenElement>(770 - 300, 394 - 120 + 8, 1150, 394 - 120 + 8 + 60 + 2, "bigbeige.png", target));

		if (!file_dialog)
		{
			auto files = make_unique<Files>(770.0 - 300, 404.0 - 60, 1150.0, 744.0 - 60, "white.png", this, target);
			auto files_ptr = files.get();
			to_be_added.insert(to_be_added.end() - 1, std::move(files));
			files_ptr->enter_directory(0);
			files_ptr->file_open_function = [this, files_ptr](string file_name)
			{
				this->start_application(PAINT);
				Canvas* canvas;
				for (int i = 0; i < this->to_be_added.size(); ++i)
				{
					if (canvas = dynamic_cast<Canvas*>(this->to_be_added[i].get()))
						canvas->load_image_file(file_name, files_ptr->current_folder);
				}
			};
		}

		translate_application(to_be_added, -235, 125, FILES);
		break;
	}

	case CONSOLE:
	{
		if (update_in_progress)
		{
			start_application(INFO_BOX, "Cannot use system services while system update is in progress.");
			focus_application = INFO_BOX;
			applications_open--;
			return;
		}

		auto minimize = minimize_function(CONSOLE, "icon_console_tray.png");
		auto tray = tray_function(CONSOLE, "icon_console_tray.png");
		to_be_added.push_back(make_unique<Button>(65.0 * applications_open + 15, 0.0, (65.0 * applications_open + 15) + 60, 50.0, "icon_console_tray.png", CONSOLE, tray, [](){}, "", "sheen.png", "sheen2.png"));
		borders_title_x(570.0, 404.0, 1350.0, 744.0, CONSOLE);
		//to_be_added.push_back(make_unique<ScreenElement>(560.0, 744.0, 1360.0, 780.0, "silver.png", CONSOLE));
		to_be_added.push_back(make_unique<ScreenElement>(580, 744 + 5, 600, 744 + 25, "|icon_console.png", CONSOLE));
		to_be_added.push_back(make_unique<ScreenElement>(605, 747, 605 + 125, 747 + 25, "console_name.png", CONSOLE));
		//to_be_added.push_back(make_unique<XButton>(1322.0, 747.0, 1353.0, 778.0, "xbutton.png", this, CONSOLE));
		//to_be_added.push_back(make_unique<Button>(1322.0, 747.0, 1353.0, 778.0, "xbutton.png", CONSOLE, [this]() {this->close_application(CONSOLE);}, []() {}, "", "xbuttonlight.png", "xbuttondark.png"));
		to_be_added.push_back(make_unique<Button>(1350 - 81 - 18 - 98, 744, 1350 - 81 - 18, 744 + 30, "minimize.png", CONSOLE, minimize, [](){}, "", "minimizelight.png", "minimizedark.png"));
		/*to_be_added.push_back(make_unique<ScreenElement>(560.0, 394.0, 570.0, 744.0, "silver.png", CONSOLE));
		to_be_added.push_back(make_unique<ScreenElement>(1350.0, 394.0, 1360.0, 744.0, "silver.png", CONSOLE));
		to_be_added.push_back(make_unique<ScreenElement>(560.0, 394.0, 1360.0, 404.0, "silver.png", CONSOLE));*/
		to_be_added.push_back(make_unique<Console>(570.0, 404.0, 1350.0, 744.0, "black.png", this));
		to_be_added.back()->surrender_focus_on_up_click = false;
		translate_application(to_be_added, 400, -50, CONSOLE);
		queue_click_x = scalex(1000);
		queue_click_y = scaley(1080 - 500);
		break;
	}

	case INFO_BOX:
	{
		if (download_action == 0 && scroll_panel == NULL && !progress_dialog)
			sound_manager->play_sound("boxbeep.wav");

		reset_elements();
		if (scroll_panel != NULL)
		{
			to_be_added.push_back(make_unique<InfoText>(560.0, 470.0, 560 + 360, 470 + 200, "browserdialog.png", info_text));
			to_be_added.push_back(make_unique<Button>(660, 475, 660 + 150, 475 + 40, "browserok.png", INFO_BOX, [this, scroll_panel]()
			{
				this->close_application(INFO_BOX);
				this->greyed_out = false;
			}, [](){}, "", "browserheld.png", "browserpressed.png"));
			auto deltas = to_be_added[to_be_added.size() - 2]->center();
			to_be_added[to_be_added.size() - 1]->translate(deltas.first, deltas.second);
			tag_info_box();
			return;
		}

		focus_application = INFO_BOX;
		if (download_action == 100)
		{
			to_be_added.push_back(make_unique<ScreenElement>(800, 300, 800 + 680, 300 + 329, "infobg.png", INFO_BOX));
			borders_title_x(800, 300, 800 + 680, 300 + 329, INFO_BOX);
			to_be_added.push_back(make_unique<Animation>(820, 340, -1, -1, "lock", INFO_BOX, 60, true));
			to_be_added.push_back(make_unique<ScreenElement>(850 + 196, 300 + 166, -1, -1, "encryptionlabel.png", INFO_BOX));
			auto ok_button = make_unique<Button>(850 + 196, 300 + 106, -1, -1, "ok.png", INFO_BOX, [this]() {this->start_application(INFO_BOX, "Password incorrect.");}, []() {});
			auto ok_button_ptr = ok_button.get();
			to_be_added.push_back(std::move(ok_button));
			auto text_field = make_unique<TextField>(850 + 196 + 73, 300 + 166 - 2, 850 + 196 + 73 + 225, 300 + 166 + 30 - 2, "systemtextfield.png", this, INFO_BOX, [ok_button_ptr]() {ok_button_ptr->release_function();});
			text_field->translate(0, -5);
			text_field->password = true;
			to_be_added.push_back(std::move(text_field));
			tag_info_box();
			return;
		}

		if (!progress_dialog)
		{
			auto string_widths = draw_string(texture_manager, 32, info_text, 0, 0, false);
			float width = max(string_widths.back(), (float)162.0);
			auto white = make_unique<ScreenElement>(0, 0, width + 20, 100, "infobg.png", INFO_BOX);
			white->center();
			borders_title_x(white->x1, white->y1, white->x2, white->y2, INFO_BOX);
			auto text = make_unique<InfoText>(white->x1, white->y1 + 65, white->x1, white->y1 + 65, "invisible.png", info_text);
			auto button = make_unique<Button>(0, 0, 0, 0, "ok.png", INFO_BOX, [this]() 
			{
				this->close_application(INFO_BOX); 
				this->dialogue_confirm_function(); 
				this->dialogue_confirm_function = []() {}; 
			}, []() {}, "", "", "", "", false, true);
			button->center();
			button->translate(0, white->y1 + 10 - button->y1);
			to_be_added.push_back(std::move(white));
			to_be_added.push_back(std::move(text));
			to_be_added.push_back(std::move(button));
			tag_info_box();
			return;
		}
		
		float whitex1 = 300;
		float whitex2 = 600;
		float whitey1 = 100;
		float whitey2 = 250;
		auto whitebg = make_unique<InfoText>(whitex1, whitey1, whitex2, whitey2, "infobg.png", info_text);
		whitebg->center();
		auto white = whitebg.get();
		to_be_added.push_back(std::move(whitebg));
		borders_title_x(white->x1, white->y1, white->x2, white->y2, INFO_BOX, false);
		if (!progress_dialog)
		{
			to_be_added.push_back(make_unique<Button>((whitex1 + whitex2) / 2 - 81, 500.0, (whitex1 + whitex2) / 2 + 81, 500 + 43, "ok.png", INFO_BOX, [this]() {this->close_application(INFO_BOX);}, []() {}));
		}

		else
			to_be_added.push_back(make_unique<ProgressBar>(white->x1 + 15, white->y1 + 15, white->x2 - 15, white->y1 + 40, "shinygrey.png", this, INFO_BOX, [this, download_action]()
		{
			if (download_action == 0)
			{
				if (extra_apps.size() == 0)
				{
					this->extra_apps.push_back(pair<string, Application>("icon_paint_text.png", PAINT));
					this->screen_elements.insert(screen_elements.begin() + get_internet_icon(), make_unique<DesktopIcon>(50.0, 600.0, 100.0, 656.0, "icon_paint_text.png", this, PAINT));
					save_game(save_slot);
				}
			}

			else if (download_action == 1)
			{
				if (extra_apps.size() == 1)
				{
					this->extra_apps.push_back(pair<string, Application>("icon_video.png", VIDEO_APP));
					this->screen_elements.insert(screen_elements.begin() + get_internet_icon(), make_unique<DesktopIcon>(50.0, 500.0, 100.0, 550.0, "icon_video.png", this, VIDEO_APP));
				}
			}

			else if (download_action == 2)
			{
				if (extra_apps.size() == 1)
				{
					this->extra_apps.push_back(pair<string, Application>("icon_decrypt_text.png", DECRYPTION_APP));
					this->screen_elements.insert(screen_elements.begin() + get_internet_icon(), make_unique<DesktopIcon>(48, 500, 102, 556.0, "icon_decrypt_text.png", this, DECRYPTION_APP));
				}
			}

			else if (download_action == 3)
			{
				if (extra_apps.size() == 1)
				{
					this->extra_apps.push_back(pair<string, Application>("icon_mmo.png", MMO_GAME));
					this->screen_elements.insert(screen_elements.begin() + get_internet_icon(), make_unique<DesktopIcon>(50, 400, 100, 450, "icon_mmo.png", this, MMO_GAME));
				}
			}

			else if (download_action == 4)
			{
				File compiler;
				compiler.name = "bcc";
				compiler.type = 1;
				write_file_to_folder(compiler, "xj5555585");
			}

			else if (download_action == 5)
			{
				File tut1;
				tut1.name = "lesson1";
				tut1.type = 2;
				write_file_to_folder(tut1, "xj5555585");
			}

			else if (download_action == 6)
			{
				File tut2;
				tut2.name = "lesson2";
				tut2.type = 2;
				write_file_to_folder(tut2, "xj5555585");
			}

			//TODO restricted folder permissions?
			else if (download_action == 7)
			{
				File tut3;
				tut3.name = "lesson3";
				tut3.type = 2;
				write_file_to_folder(tut3, "xj5555585");
			}
		}));

		if (progress_dialog && download_action == 0)
			to_be_added.push_back(make_unique<ScreenElement>(white->x1 + 15, white->y1 + 140 - 50, white->x1 + 65, white->y1 + 190 - 50, "paintbrush.png", INFO_BOX));

		if (progress_dialog && download_action == 1)
			to_be_added.push_back(make_unique<ScreenElement>(white->x1 + 15, white->y1 + 140 - 50, white->x1 + 65, white->y1 + 190 - 50, "|icon_video.png", INFO_BOX));

		if (progress_dialog && download_action == 2)
			to_be_added.push_back(make_unique<ScreenElement>(white->x1 + 15, white->y1 + 140 - 50, white->x1 + 65, white->y1 + 190 - 50, "|icon_decrypt.png", INFO_BOX));

		tag_info_box();
		break;
	}
	
	//TODO use tray function
	case PAINT:
	{
		int window_title_base = 1080 - WINDOW_TITLE_HEIGHT;
		auto minimize = minimize_function(PAINT, "icon_paint_tray.png");
		to_be_added.push_back(make_unique<Button>(65.0 * applications_open + 15, 0.0, (65.0 * applications_open + 15) + 60.0, 50.0, "icon_paint_tray.png", PAINT, minimize_function(PAINT, "icon_paint_tray.png"), [](){}, "", "sheen.png", "sheen2.png"));
		to_be_added.push_back(make_unique<ScreenElement>(0.0, window_title_base, 1920.0, 1080.0, "silver.png", PAINT, false, true));
		to_be_added.push_back(make_unique<ScreenElement>(0.0, 980.0 + 6, 1920.0, 1044.0 + 6, "beige.png", PAINT));
		//to_be_added.push_back(make_unique<XButton>(1882.0, 1047.0, 1913.0, 1078.0, "xbutton.png", this, PAINT));
		to_be_added.push_back(make_unique<Button>(1920 - 81, window_title_base, 1920, 1078.0 + 2, "xbutton.png", PAINT, [this]() {this->close_application(PAINT);}, []() {}, "", "xbuttonlight.png", "xbuttondark.png"));
		to_be_added.push_back(make_unique<Button>(1920 - 81 - 18 - 98, window_title_base, 1920 - 81 - 18, 1078.0 + 2, "minimize.png", PAINT, minimize, [](){}, "", "minimizelight.png", "minimizedark.png"));
		to_be_added.push_back(make_unique<ScreenElement>(0.0, 56.0, 1920.0, 980.0 + 6, "paintbackground.png", PAINT));
		//to_be_added.push_back(make_unique<Canvas>(200.0, 200.0, 800.0, 800.0, "white.png", this, PAINT));
		auto canvas = make_unique<Canvas>(20.0, 200.0 + 6, 800.0, 960.0 + 6, "white.png", this, PAINT);
		Canvas* canvas_ptr = canvas.get();
		canvas->always_inform_mouse_moved = true;
		to_be_added.push_back(std::move(canvas));
		to_be_added.push_back(make_unique<Resizer>(0, 0, 0, 0, "resizer.png", PAINT, canvas_ptr, 0, this));
		to_be_added.push_back(make_unique<Resizer>(0, 0, 0, 0, "resizer.png", PAINT, canvas_ptr, 1, this));
		to_be_added.push_back(make_unique<Resizer>(0, 0, 0, 0, "resizer.png", PAINT, canvas_ptr, 2, this));
		vector<unique_ptr<Button>> buttons;
		/*int x1 = 260;
		int y1 = 990;
		int y2 = 1040;
		int x2 = 310;*/
		int x1 = 260;
		int y1 = 1020 - 30;
		int x2 = 390;
		int y2 = 1040 - 30;
		vector<string> tool_names = { "brush", "eraser", "fill", "line", "stickylines", "rectangle", "filledrectangle", "ellipse", "filledellipse", "colorpicker", "select", "undo", "save", "open", "help" };
		for (int i = 0; i < tool_names.size(); ++i)
		{
			if (i % 2 == 0)
			{
				y1 += 30;
				y2 += 30;
			}

			else
			{
				y1 -= 30;
				y2 -= 30;
			}

			string tool_name = tool_names[i];
			auto button = make_unique<Button>(x1, y1, x2, y2, "invisible.png", PAINT, [canvas_ptr, tool_name]() {canvas_ptr->change_tool(tool_name);}, []() {}, "", "bookmarkcover.png", "bookmarkcoverdark.png", "", true);
			button->note = tool_names[i];
			if (tool_name == "help")
				button->tooltip = "helptip.png";

			buttons.push_back(std::move(button));
			to_be_added.push_back(make_unique<ScreenElement>(x1, y1, x2, y2, "label" + tool_names[i] + ".png", PAINT));
			if (i % 2 != 0)
			{
				x1 += 155;
				x2 += 155;
			}
		}

		buttons[buttons.size() - 3]->release_function = [canvas_ptr]() {canvas_ptr->save_file();};
		buttons[buttons.size() - 2]->release_function = [canvas_ptr]() {canvas_ptr->load_file();};
		buttons[0]->dark = true;
		for (int i = buttons.size() - 1; i >= 0; --i)
		{
			canvas_ptr->tools.push_back(buttons[i].get());
			to_be_added.push_back(std::move(buttons[i]));
		}

		to_be_added.push_back(make_unique<ScreenElement>(10.0, window_title_base + 5, 30.0, 1080 - 5, "|icon_paint.png", PAINT));
		to_be_added.push_back(make_unique<ScreenElement>(35, window_title_base + 3, 35 + 125, 1080 - 2, "paint_name.png", PAINT));

		//to_be_added.push_back(make_unique<MMO>(0.0, 56.0, 1920.0, 1044.0, "black.png", this, PAINT));
		// // auto mmo = make_unique<MMO>(0.0, 56.0, 1920.0, 1044.0, "black.png", this, PAINT);
		// // to_be_added.insert(to_be_added.end() - 3, std::move(mmo));
		// auto help_center = make_unique<HelpCenter>(0.0, 56.0, 1920.0, 1044.0, "white.png", this, PAINT);
		// to_be_added.insert(to_be_added.end() - 2, std::move(help_center));
		// unique_ptr<NumberField> number_field = make_unique<NumberField>(300.0, 300.0, 500.0, 400.0, "textfield.png", this, PAINT);
		// to_be_added.insert(to_be_added.end() - 2, std::move(number_field)); //2 is because the NumberField will add two children it needs to be beneath
		break;
	}

	case VIDEO_APP:
	{
		screen_elements.insert(screen_elements.end(), make_unique<ShoppingCenter>(0, 0, 0, 0, "", VIDEO_APP, this));
		//screen_elements.insert(screen_elements.end(), make_unique<ScreenElement>(0, 0, 1920, 1080, "black.png", VIDEO_APP));
		//screen_elements.insert(screen_elements.end(), make_unique<Video>(0, 540, 960, 1080, "blue.png", VIDEO_APP, this));
		break;
	}

	case DECRYPTION_APP:
	{
		auto tray = tray_function(DECRYPTION_APP, "icon_decrypt_tray.png");
		auto button = make_unique<Button>(65.0 * applications_open + 15, 0.0, (65.0 * applications_open + 15) + 60.0, 50.0, "icon_decrypt_tray.png", DECRYPTION_APP, tray, []() {}, "", "sheen.png", "sheen2.png");
		auto button_ptr = button.get();
		to_be_added.push_back(std::move(button));
		auto decrypt = make_unique<Decryption>(500, 500, 1000, 1000, this, DECRYPTION_APP);
		auto decrypt_ptr = decrypt.get();
		to_be_added.push_back(std::move(decrypt));
		button_ptr->release_function = [decrypt_ptr, tray]()
		{
			if (decrypt_ptr->screen_number != 0)
				tray();
		};

		break;
	}

	case MMO_GAME:
	{
		auto mmo = make_unique<MMO>(0, 0, 1920, 1080, "black.png", "testboxes.txt", this, MMO_GAME); //map1.png.txt
		MMO* mmo_ptr = mmo.get();
		screen_elements.insert(screen_elements.end(), std::move(mmo));
		//mmo_ptr->add_children(); TODO uncomment for password screen and change app_state to 0
		mmo_ptr->add_chat();
		break;
	}

	case LEVEL_EDITOR_APP:
	{
		auto editor = make_unique<LevelEditor>(0, 0, 1920, 1080, "map1.png", this, LEVEL_EDITOR_APP);
		screen_elements.insert(screen_elements.end(), std::move(editor));
		break;
	}
	}

	if (application != INFO_BOX)
		start_open_animation(application);
}

void Computer::translate_application(vector<unique_ptr<ScreenElement>>& vec, float delta_x, float delta_y, Application application, bool direct)
{
	if (direct)
		get_extent(application, vec); //TODO can you trigger this while minimizing? Can you start two minimize animations at the same time?

	for (int i = 0; i < vec.size(); ++i)
	{
		if (application == INFO_BOX && vec[i]->info_box_id != info_box_id)
			continue;

		if (vec[i]->application == application && vec[i]->name.substr(0, 5) != "icon_")
		{
			if (!direct)
				vec[i]->translate(delta_x, delta_y);

			else
				vec[i]->translate(delta_x - min_x1, delta_y - min_y2);
		}
	}
}

void Computer::get_extent(Application application, vector<unique_ptr<ScreenElement>>& target)
{
	min_x1 = -1; min_x2 = -1; min_y1 = -1; min_y2 = -1; //TODO is this good?
	if (application == BROWSER)
	{
		min_x1 = 0;
		min_y1 = 0;
		min_x2 = 1920;
		min_y2 = 1080;
		return;
	}

	for (int i = 0; i < target.size(); ++i)
	{
		auto se = target[i].get();
		if (se->application != application || se->name.substr(0, 5) == "icon_")
			continue;

		if ((se->x1 < min_x1 || min_x1 == -1))
			min_x1 = se->x1;

		if ((se->x2 > min_x2 || min_x2 == -1))
			min_x2 = se->x2;

		if ((se->y1 < min_y1 || min_y1 == -1))
			min_y1 = se->y1;

		if ((se->y2 > min_y2 || min_y2 == -1))
			min_y2 = se->y2;
	}
}

void Computer::setup_minimizing_buffer()
{
	glGenFramebuffers(1, &minimizing_framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, minimizing_framebuffer);
	glGenTextures(1, &minimizing_texture);
	glBindTexture(GL_TEXTURE_2D, minimizing_texture); //TODO I hope this doesn't break any assumptions about textures
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, min_x2 - min_x1, min_y2 - min_y1, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0); //TODO cuts off one pixel?  Have I done this everywhere?
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //TODO check these
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, minimizing_texture, 0);
	glGenRenderbuffers(1, &minimizing_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, minimizing_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, min_x2 - min_x1, min_y2 - min_y1);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, minimizing_rbo);
	//GLenum draw_buffers[1] = { GL_COLOR_ATTACHMENT0 };
	//glDrawBuffers(1, draw_buffers); TODO apparently this did nothing
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Computer::start_open_animation(Application application)
{
	minimizing_alpha = 0.0;
	play_minimizing_animation = true;
	minimize_mode = 1;
	minimizing_application = application;
	get_extent(application, to_be_added);
	min_x1 = scalex(min_x1);
	min_x2 = scalex(min_x2);
	min_y1 = scaley(min_y1);
	min_y2 = scaley(min_y2);
	cur_minimize_x1 = min_x1 + 10;
	cur_minimize_y1 = min_y1 + 10;
	setup_minimizing_buffer();
}

void Computer::start_close_animation(Application application)
{
	minimizing_alpha = 1.0;
	play_minimizing_animation = true;
	minimize_mode = 2;
	minimizing_application = application;
	get_extent(application, screen_elements);
	min_x1 = scalex(min_x1);
	min_x2 = scalex(min_x2);
	min_y1 = scaley(min_y1);
	min_y2 = scaley(min_y2);
	cur_minimize_x1 = min_x1;
	cur_minimize_y1 = min_y1;
	setup_minimizing_buffer();
	glEnable(GL_TEXTURE_2D);
	draw_desktop();
}

std::function<void()> Computer::minimize_function(Application application, string icon_name, bool play_animation)
{
	return [this, application, icon_name, play_animation]()
	{
		int start = -1;
		int end;
		int desktop;
		for (int i = 0; i < screen_elements.size(); ++i)
		{
			if (screen_elements[i]->application == application)
			{
				if (start == -1)
					start = i;
			}

			else
			{
				if (start != -1)
				{
					end = i;
					break;
				}
			}
		}

		for (int i = 0; i < screen_elements.size(); ++i)
		{
			if (screen_elements[i]->name == "background.png")
				desktop = i;
		}

		if (end > desktop)
		{
			if (!everything_stuck)
				move_range(screen_elements, start, end, 0);

			alpha_going_up = false;
			minimizing_alpha = 1.0;
		}

		else
		{
			if (!everything_stuck)
				move_range(screen_elements, start, end, -2 - applications_open);

			alpha_going_up = true;
			minimizing_alpha = 0.0;
		}
		
		if (!play_animation)
			return;
		
		play_minimizing_animation = true;
		minimize_mode = 0;
		minimizing_application = application;
		get_extent(application, screen_elements);
		min_x1 = scalex(min_x1);
		min_x2 = scalex(min_x2);
		min_y1 = scaley(min_y1);
		min_y2 = scaley(min_y2);
		int other_apps = 0;
		for (int i = screen_elements.size() - 1; i >= 0; i--)
		{
			if (screen_elements[i]->name == icon_name)
				break;

			if (screen_elements[i]->name.substr(0, 5) == "icon_")
				other_apps++;
		}

		target_minimize_x = (applications_open - other_apps) * 50 + 25;
		
		if (!alpha_going_up)
		{
			cur_minimize_x1 = min_x1;
			cur_minimize_y1 = min_y1;
		}

		else
		{
			cur_minimize_x1 = (target_minimize_x - min_x1) * 0.75 + min_x1;
			cur_minimize_y1 = (0 - min_y1) * 0.75 + min_y1;
		}

		setup_minimizing_buffer();
	};
}

bool Computer::foreground_if_obscured(Application application, string icon_name)
{
	bool obscured = false;
	bool reached_desktop = false;
	for (int i = 0; i < screen_elements.size(); ++i)
	{
		if (screen_elements[i]->name == "background.png")
			reached_desktop = true;

		if (screen_elements[i]->application == application)
		{
			if (!reached_desktop)
				continue;

			for (int j = i + 1; j < screen_elements.size() - applications_open - 2; ++j)
			{
				if (screen_elements[j]->application != application && intersects(*(screen_elements[i].get()), *(screen_elements[j].get())))
					obscured = true;
			}
		}
	}

	if (obscured && !everything_stuck)
	{
		auto minimize = minimize_function(application, icon_name, false);
		minimize();
		if (screen_elements[0]->application == application)
			minimize();
	}

	return obscured;
}

function<void()> Computer::tray_function(Application application, string icon_name) //TODO does this interact with breaking ok?
{
	return [this, application, icon_name]()
	{
		bool obscured = foreground_if_obscured(application, icon_name);
		if (!obscured)
		{
			auto minimize = minimize_function(application, icon_name);
			minimize();
		}
	};
}

void Computer::start_playing_escape_sound(string escape_sound)
{
	if (playing_escape_sound)
		return;

	playing_escape_sound = true;
	sound_manager->current_escape_sound = escape_sound;
	sound_manager->play_sound(escape_sound);
}

void Computer::stop_website_sounds() //TODO make sure all sounds are here
{
	sound_manager->smooth_stop("ambient.wav");
	sound_manager->smooth_stop("title.wav");
	sound_manager->smooth_stop("takeoff.wav");
	sound_manager->smooth_stop("bomb.wav");
	sound_manager->smooth_stop("main.wav");
	sound_manager->smooth_stop("gameover.wav");
	sound_manager->smooth_stop("laser.wav");
}

int Computer::get_desktop_index()
{
	for (int i = 0; i < screen_elements.size(); ++i)
	{
		if (screen_elements[i]->name == "background.png")
			return i;
	}

	return 0;
}

void Computer::reset_elements()
{
	for (int i = 0; i < screen_elements.size(); ++i)
	{
		screen_elements[i]->mouse_off();
		screen_elements[i]->take_focus();
	}

	focus_element = screen_elements[get_desktop_index()].get();
}

void Computer::remove_deleted()
{
	screen_elements.erase(remove_if(screen_elements.begin(), screen_elements.end(), [this](unique_ptr<ScreenElement>& elem)
	{
		if (elem->marked_for_deletion)
        {
			//cout << "deleting " + elem->name + "\n";
			if (elem.get() == focus_element)
                focus_element = screen_elements[get_desktop_index()].get();
            
            return true;
        }
        
        return false;
	}), screen_elements.end());
}

void Computer::close_application(Application application)
{
	if (application == BROWSER)
		stop_website_sounds();
	
	if (application != INFO_BOX)
	{
		applications_open--;
		start_close_animation(application);
		if (everything_stuck)
		{
			applications_open++;
			close_bucket++;
			return;
		}
	}

	if (application == DECRYPTION_APP)
		decrypt_forcing = false;

	if (application == INFO_BOX && started_crashing_browser && !crashed_browser)
	{
		close_application(BROWSER);
		crashed_browser = true;
		update_in_progress = false;
		save_game(save_slot);
	}

	if (application == VIDEO_APP)
	{
		extra_apps.erase(extra_apps.end() - 1);
		for (int i = 0; i < screen_elements.size(); ++i)
		{
			if (screen_elements[i]->name == "icon_video.png")
				screen_elements[i]->marked_for_deletion = true;
		}

		ran_trial = true;
		start_application(INFO_BOX, "The trial will now uninstall itself from your computer.");
		//delist("download_trial"); this did nothing
		websites["trialexpired"] = { "trialexpired.png" };
		save_game(save_slot);
	}

	for (int i = 0; i < screen_elements.size(); ++i)
	{
		ScreenElement* se = screen_elements[i].get();
		if (se->application == application)
		{
			if (application == INFO_BOX)
			{
				if (se->info_box_id == info_box_id)
					se->marked_for_deletion = true;
			}
			
			else se->marked_for_deletion = true;
		}
	}

	if (application == INFO_BOX)
		flash_counter = -1;

	if (restore_application != NONE && info_box_id <= 1)
	{
		focus_application = restore_application;
		restore_application = NONE;
	}

	if (application == INFO_BOX)
		info_box_id--;

	int application_number = 0;
	for (int i = 0; i < screen_elements.size(); ++i)
	{
		if (screen_elements[i]->name.substr(0, 5) == "icon_" && screen_elements[i]->y1 == 0.0 && !screen_elements[i]->marked_for_deletion)
		{
			application_number++;
			if (screen_elements[i]->x1 != 65.0 * application_number + 15)
				screen_elements[i]->smooth_translate(65.0 * application_number + 15 - screen_elements[i]->x1, 0);
		}
	}
}

void Computer::tag_info_box()
{
	info_box_id++;
	for (int i = 0; i < to_be_added.size(); ++i)
		to_be_added[i]->info_box_id = info_box_id;
}

bool Computer::file_present(string folder, string file_name)
{
	auto list_of_files = get_folder_index(folder);
	for (int i = 0; i < list_of_files.size(); ++i)
	{
		if (list_of_files[i].name == file_name)
			return true;
	}

	return false;
}

void Computer::make_everything_stuck()
{
	everything_stuck = true;
}

void Computer::close_for_decrypt()
{
	decrypt_forcing = true;
	vector<Application> to_close = { BROWSER, FILES, CONSOLE, PAINT };
	for (int i = 0; i < to_close.size(); ++i)
	{
		for (int j = 0; j < screen_elements.size(); ++j)
		{
			if (screen_elements[j]->application == to_close[i] && screen_elements[j]->name.substr(0, 5) != "icon_")
			{
				close_application(to_close[i]);
				break;
			}
		}
	}
}

void Computer::press_key(unsigned char key)
{
	/*if (key == 'k')
	{
		go_to_flying = true;
		mmo_stage_x1 = 0;
		mmo_stage_y1 = 0;
		mmo_player_x1 = 0;
		mmo_player_y1 = 0;
	}*/

	if (freeze_time > 0)
		return;

	if (key == 27)
	{
		display_pause_screen();
		return;
	}

	/*if (key == '[')
	{
		cout << "pmx " << prev_mouse_x << "\n";
		cout << "pmy " << prev_mouse_y << "\n";
	}*/
	
	/*if (key == ']')
	{
		no_mouse = true;
		return;
	}
	
	if (key == '[')
	{
		start_application(LEVEL_EDITOR_APP);
		return;
	}

	if (key == '-')
	{
		cout << "-----------" << "\n";
		for (int i = 0; i < screen_elements.size(); ++i)
			cout << i << " " << screen_elements[i]->name << "\n";
	}

	if (key == '&')
		score_table.back().second = 5000;*/
	
	if (focus_element != NULL)
	{
		if (focus_element->browser_child && (key == 21 || key == 23))
		{
			for (int i = 0; i < screen_elements.size(); ++i)
			{
				ScrollPanel* sp = dynamic_cast<ScrollPanel*>(screen_elements[i].get());
				if (sp != NULL)
					sp->press_key(key);
			}
		}

		else focus_element->press_key(key);
	}

	process_control_changes();
}

void Computer::release_key(unsigned char key)
{
	if (freeze_time > 0)
		return;
	
	if (focus_element != NULL)
	{
		if (focus_element->browser_child && (key == 21 || key == 23))
		{
			for (int i = 0; i < screen_elements.size(); ++i)
			{
				ScrollPanel* sp = dynamic_cast<ScrollPanel*>(screen_elements[i].get());
				if (sp != NULL)
					sp->release_key(key);
			}
		}

		else focus_element->release_key(key);
	}
}

void Computer::set_computer_done(bool val)
{
	computer_done = val;
}

bool Computer::is_computer_done()
{
	return computer_done;
}

Computer::~Computer()
{
	if (uploaded_image.image != NULL)
		free(uploaded_image.image);
}