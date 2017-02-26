#include <algorithm>
#include <fstream>
#include "ScrollPanel.h"
#include "ScrollBar.h"
#include "ObjModel.h"
#include "RadioButton.h"
#include "TextField.h"
#include "Button.h"
#include "HelpCenter.h"
#include "Files.h"
#include "StaticCanvas.h"
#include "BubbleGame.h"
#include "MMO.h"
#include "Slideshow.h"
#include "InfoText.h"
#include "Spinner.h"
#include "Blocks.h"
#include "ExpandableImage.h"
#include "keymappings.h"

using namespace std;

//TODO: Scrolling slows down when moving the mouse
//TODO up and down buttons stopped working, holding them down does nothing
//TODO lots of screen gets cut off when minimizing forumhome and reddit
//TODO see hidden images when minimizing AIN
//TODO shrink scrollbar so it's beneath bookmark toolbar, especially to prevent scrolling during space game
//TODO should not be able to upclick to activate links

ScrollPanel::ScrollPanel(float _x1, float _y1, float _x2, float _y2, Computer* _parent, Application _application, bool no_extras) : ScreenElement(_x1, _y1, _x2, _y2, "", _application), parent(_parent), offset(0), timebase(-1), going_up(false)
{
	do_whitelist_check = false;
	animation_function = []() {};
	if (!no_extras)
		parent->to_be_added.push_back(make_unique<ScreenElement>(0.0, 1000.0, 1900.0, 1044.0, "beige.png", BROWSER));
	
	auto text_field = make_unique<TextField>(100.0, 1004.0, 700.0, 1034.0, "textfield.png", parent, BROWSER);
	text_field_ptr = text_field.get();
	auto button = make_unique<Button>(720.0, 1004.0, 750.0, 1034.0, "go.png", BROWSER,
		[this]()
	{
		this->start_loading_site(this->text_field_ptr->text, false);
	}, [](){}, "", "golight.png", "godark.png");
	auto button_ptr = button.get();
	text_field_ptr->on_enter_function = [button_ptr]() { button_ptr->release_function(); };
	parent->to_be_added.push_back(std::move(text_field));
	parent->to_be_added.push_back(std::move(button));
	
	if (!no_extras)
	{
		parent->to_be_added.push_back(make_unique<Button>(665.0, 1009.0, 695.0, 1029.0, "history.png", BROWSER, []() {},
			[this]()
		{
			for (int i = history.size() - 1; i >= 0 && history.size() - i < 6; --i)
			{
				auto history_button = make_unique<Button>(100.0, text_field_ptr->y1 - (75.0 * (history.size() - i)), 700.0, text_field_ptr->y1 - (75.0 * (history.size() - i - 1)), "white.png", BROWSER,
					[this, i]()
				{
					this->change_website(this->history[i]);
				}, []() {}, history[i]);
				history_button->transient = true;
				parent->to_be_added.push_back(std::move(history_button));
			}
		}));
		parent->to_be_added[parent->to_be_added.size() - 1]->is_visible = false;
	}

	if (no_extras)
	{
		text_field_ptr->is_visible = false;
		button_ptr->is_visible = false;
	}
	
	websites["search"] = { "search.png" };
	websites["reddit"] = { "sleepy.png" };
	websites["tablet"] = { "sleepy.png" };
	websites["conversation"] = { "sleepy.png" };
	websites["tomthread"] = { "sleepy.png" };
	websites["useful"] = { "useful1.png" };
	websites["nenda"] = { "nendablank.png" };
	websites["rfh2"] = { "rfh2.png" };
	websites["rfh1"] = { "rfh1.png" };
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
	websites["ain"] = { "ain1.png" };
	websites["e1"] = { "e1.png" };
	websites["psych"] = { "psych.png" };
	websites["contestdesc"] = { "contestdesc.png" };
	websites["thankyouforuploading"] = { "thankyouforuploading.png" };
	websites["centerracom"] = { "centerracomhomepage.png" };
	websites["centerracomtrial"] = { "centerracomtrial.png" };
	websites["centerracomthankyou"] = { "centerracomthankyou.png" };
	websites["triallink"] = { "triallink.png" };
	websites["surveylink"] = { "surveylink.png" };
	websites["hammercounters1"] = { "e1.png" };
	websites["decrypt"] = { "e1.png" };
	websites["e2"] = { "e1.png" };
	websites["helphome"] = { "helphome.png" };
	websites["forumhome"] = { "forumhome.png" };
	websites["contestresults"] = { "e1.png" };
	websites["game"] = { "bigwhite.png" };
	websites["weirdstuff"] = { "e1.png" };
	websites["mmo"] = { "e1.png" };
	websites["recoveremail"] = { "recoveremail.png" };
	websites["recoverpassword"] = { "e1.png" };
	websites["psychologist"] = { "psychologist.png" };
	websites["spam"] = { "nendaspam.png" };
	websites["trash"] = { "nendatrash.png" };
	websites["about"] = { "nendaabout.png" };
	websites["search/search?q="] = { "searchspinner.png" };

	sequences["sequencereddit"] = { "tomthread", "tablet", "conversation" };

	alternative_links["nendaspam.png"] = "nendablank.link";
	alternative_links["nendatrash.png"] = "nendablank.link";
	alternative_links["nendaabout.png"] = "nendablank.link";
	alternative_links["rfh1.png"] = "nendablank.link";
	alternative_links["rfh2.png"] = "nendablank.link";
	alternative_links["forumemail1.png"] = "nendablank.link";
	alternative_links["forumemail2.png"] = "nendablank.link";
	alternative_links["deliverfailure.png"] = "nendablank.link";

	layout["forumhome"] = { pair<int, string>(0, "header.png"), pair<int, string>(0, "header.png"), pair<int, string>(0, "header.png"), pair<int, string>(0, "header.png"), pair<int, string>(0, "header.png") };
	vector<pair<int, string>> reddit_layout;
	for (int i = 0; i < 20; ++i)
		reddit_layout.push_back(pair<int, string>(0, "itemtitle.png"));

	reddit_layout.push_back(pair<int, string>(-738, "sleepyfooter.png"));

	vector<pair<int, string>> search_layout;
	for (int i = 0; i < 10; ++i)
		search_layout.push_back(pair<int, string>(0, "result.png"));

	search_layout.push_back(pair<int, string>(-1, "didyoulike.png"));

	vector<pair<int, string>> conversation_layout, tablet_layout, tomthread_layout;
	conversation_layout.push_back(pair<int, string>(0, "conversationtitle.png"));
	tablet_layout.push_back(pair<int, string>(0, "tablettitle.png"));
	tomthread_layout.push_back(pair<int, string>(0, "tomthreadtitle.png"));
	for (int i = 0; i < 20; ++i)
		conversation_layout.push_back(pair<int, string>(i % 2 == 0 ? 0 : 1, "comment1.png"));
	for (int i = 0; i < 20; ++i)
		tablet_layout.push_back(pair<int, string>(i % 2 == 0 ? 0 : 1, "comment2.png"));
	for (int i = 0; i < 20; ++i)
		tomthread_layout.push_back(pair<int, string>(i % 2 == 0 ? 0 : 1, "comment3.png"));
	conversation_layout.push_back(pair<int, string>(-738, "sleepyfooter.png"));
	tablet_layout.push_back(pair<int, string>(-738, "sleepyfooter.png"));
	tomthread_layout.push_back(pair<int, string>(-738, "sleepyfooter.png"));

	layout["reddit"] = reddit_layout;
	layout["searchresults"] = search_layout;
	layout["conversation"] = conversation_layout;
	layout["tomthread"] = tomthread_layout;
	layout["tablet"] = tablet_layout;

	survey_questions1 = 
	{
		{"Overall, how satisfied are you with the provided Centerracom Solutions product?",
		"Very satisfied", "Moderately satisfied", "Slightly satisfied", "Neither satisfied nor dissatisfied",
		"Slightly dissatisfied", "Moderately dissatisfied", "Very dissatisfied"},
		{"How likely are you to recommend any Centerracom Solutions products to a friend or coworker?",
		"Very likely", "Moderately likely", "Slightly likely", "Neither likely or unlikely", "Slightly unlikely",
		"Moderately unlikely", "Very unlikely"},
		{"How likely are you to recommend any Centerracom Solutions product to your boss or a member of senior management at your workplace or organization?",
		"Very likely", "Moderately likely", "Slightly likely", "Neither likely or unlikely", "Slightly unlikely",
		"Moderately unlikely", "Very unlikely"},
		{"Did the Centerracom Solutions product you tested seem to be a complete, production-ready product?",
		"It was very complete", "It was moderately complete", "It was only slightly complete", "It seemed somewhat incomplete",
		"It seemed very incomplete", "The product was broken and/or non-functional"},
		{"I can see myself using this Centerracom Solutions product at my organization or workplace...",
		"Very frequently", "Somewhat frequently", "Occasionally", "Somewhat infrequently", "Very infrequently"}
	};

	survey_questions2 =
	{
		{"On a scale of 1-10, rate how well each of the following words apply to our software:", "innovative"},
		{"", "lackadaisical"},
		{"", "performant"}
	};

	prev_mouse_x = 0;
	prev_mouse_y = 0;
    history_index = -1;
	prev_website = "";
	required_target = "";
}

bool ScrollPanel::whitelist_check(string website_name, bool force_through)
{
	if (force_through)
		return true;

	if (do_whitelist_check)
	{
		for (int i = 0; i < parent->whitelist.size(); ++i)
		{
			if (parent->whitelist[i] == website_name)
				return true;
		}

		return false;
	}

	else return true;
}

void ScrollPanel::do_layout(string website_name, int start_x, int start_y)
{
	auto cur_layout = layout[website_name];
	int cur_x = start_x;
	int cur_y = start_y;
	for (int i = 0; i < cur_layout.size(); ++i)
	{
		cur_x += cur_layout[i].first * 20;
		if (cur_layout[i].first == -1)
			cur_x = 0;

		if (cur_layout[i].first < 0 && cur_layout[i].first != -1)
			cur_x = abs(cur_layout[i].first);

		auto elem = make_unique<ScreenElement>(cur_x, cur_y - TextureManager::get_height(cur_layout[i].second), cur_x + TextureManager::get_width(cur_layout[i].second), cur_y, cur_layout[i].second, BROWSER);
		elem->never_focus = true;
		children.push_back(elem.get());
		parent->to_be_added.push_back(std::move(elem));
		string name = split(cur_layout[i].second, '.')[0];
		//string name = split(cur_layout[i].second, '.')[0] + ".link";
		name.erase(std::remove_if(name.begin(), name.end(), isdigit), name.end());
		name += ".link";
		ifstream file_stream(name);
		string line;
		while (getline(file_stream, line))
		{
			vector<string> components = split(line, ' ');
			//links.push_back(Link(Rect(atof(components[0].c_str()) + cur_x, cur_y - atof(components[1].c_str()), atof(components[2].c_str()) + cur_x, cur_y - atof(components[3].c_str())), components[4]));
			float image_height = parent->texture_manager->get_height(cur_layout[i].second);
			links.push_back(Link(Rect(atof(components[0].c_str()) + cur_x, (cur_y - image_height) + atof(components[1].c_str()), atof(components[2].c_str()) + cur_x, (cur_y - image_height) + atof(components[3].c_str())), components[4]));
		}

		cur_y -= TextureManager::get_height(cur_layout[i].second) + 20;
		cur_x = start_x;
	}

	if (cur_y < 0)
	{
		sum_of_heights -= (cur_y - 35);
		scroll_bar->is_visible = true;
		scroll_bar->y2 = 1024.0;
		float height = (1080.0 / ((float)sum_of_heights)) * 948.0;
		scroll_bar->y1 = scroll_bar->y2 - height;
	}
}

void ScrollPanel::add_children(string website_name)
{
	if (website_name == "search")
	{
		if (parent->read_email["rosecolored4bold.png"] && !parent->reached_help_center)
		{
			auto text_field = make_unique<TextField>(957 + (957 - 685), 1080 - 344, 960 + 721, (1080 - 344) + 77, "searchbar.png", parent, application);
			text_field->font_size = 64;
			text_field->text = "help";
			auto text_field_ptr = text_field.get();
			children.push_back(text_field.get());
			parent->to_be_added.push_back(std::move(text_field));
			auto button = make_unique<Button>(960 + 721, 1080 - 344, 960 + 721 + 201, (1080 - 344) + 77, "searchbutton.png", BROWSER,
				[this, text_field_ptr]()
			{
				if (text_field_ptr->text == "help")
					change_website("searchresults");

				else if (text_field_ptr->text != "")
					change_website("search/search?q=", true, true, text_field_ptr->text);
			}, [](){}, "", "genlight.png", "gendark.png");
			button->start_flashing();
			children.push_back(button.get());
			parent->to_be_added.push_back(std::move(button));
		}
		
		else
		{
			auto text_field = make_unique<TextField>(957 + (957 - 685), 1080 - 344, 960 + 721, (1080 - 344) + 77, "searchbar.png", parent, application);
			text_field->font_size = 64;;
			auto text_field_ptr = text_field.get();
			children.push_back(text_field.get());
			parent->to_be_added.push_back(std::move(text_field));
			auto button = make_unique<Button>(960 + 721, 1080 - 344, 960 + 721 + 201, (1080 - 344) + 77, "searchbutton.png", BROWSER,
				[this, text_field_ptr]()
			{
				if (text_field_ptr->text != "")
					change_website("search/search?q=", true, true, text_field_ptr->text);
			}, [](){}, "", "genlight.png", "gendark.png");
			children.push_back(button.get());
			parent->to_be_added.push_back(std::move(button));
		}

		/*auto expandable = make_unique<ExpandableImage>(500, 500, 700, 500, "testcomments.png", BROWSER, 200);
		children.push_back(expandable.get());
		auto expandable_ptr = expandable.get();
		parent->to_be_added.push_back(std::move(expandable));
		auto expand_button = make_unique<Button>(500, 600, 600, 700, "ok.png", BROWSER, [expandable_ptr]() {expandable_ptr->toggle();}, []() {});
		children.push_back(expand_button.get());
		parent->to_be_added.push_back(std::move(expand_button));*/
	}

	if (website_name == "search/search?q=")
	{
		auto spinner = make_unique<Spinner>(10, 0, 960, 1080 - 440, BROWSER);
		children.push_back(spinner.get());
		parent->to_be_added.push_back(std::move(spinner));
		animation_function = [this]()
		{
			Spinner* spinner = dynamic_cast<Spinner*>(this->children[0]);
			if (spinner != NULL)
			{
				if (spinner->frames > 150)
				{
					spinner->marked_for_deletion = true;
					children.clear();
					auto cover = make_unique<ScreenElement>(900, 1080 - 900, 1030, 1080 - 500, "white.png", BROWSER);
					auto message = make_unique<ScreenElement>(10, 900, 287, 924, "couldntfind.png", BROWSER);
					this->children.push_back(cover.get());
					this->children.push_back(message.get());
					this->parent->to_be_added.push_back(std::move(cover));
					this->parent->to_be_added.push_back(std::move(message));
					this->parent->process_control_changes();
				}
			}
		};
	}

	if (website_name == "ain")
	{
		if (parent->entered_contest)
		{
			auto button = make_unique<Button>(50.0, 100.0, 150.0, 600.0, "bannerad.png", BROWSER,
				[this]()
			{
				this->change_website("centerracom");
			}, [](){});
			children.push_back(button.get());
			parent->to_be_added.push_back(std::move(button));
		}

		auto circle = make_unique<ScreenElement>(582, 1080 - 762, 611, 1080 - 733, "aincircle.png", BROWSER);
		auto circle_ptr = circle.get();
		children.push_back(circle_ptr);
		circle_ptr->is_visible = false; //TODO make visible again
		parent->to_be_added.push_back(std::move(circle));
		vector<string> slides{ "vacation.png", "flowers.png", "fern.png", "pool.png", "coffee.png" };
		auto slideshow = make_unique<Slideshow>(235, 1080 - 567, 640, 1080 - 305, "bannerad.png", BROWSER, slides, circle_ptr);
		children.push_back(slideshow.get());
		auto slideshow_ptr = slideshow.get();
		parent->to_be_added.push_back(std::move(slideshow));
		for (int i = 0; i < 5; ++i)
		{
			auto slide_button = make_unique<Button>(591 + i * 84, 1080 - 755, (591 + i * 84) + 9, 1080 - 738, "invisible.png", BROWSER, []() {}, [slideshow_ptr, i]()
			{
				slideshow_ptr->animate_to(i);
			});
			children.push_back(slide_button.get());
			parent->to_be_added.push_back(std::move(slide_button));
		}

		vector<string> empty_labels = { "", "", "", "", "" };
		create_radio_button_group(737, 1080 - 861, 5, false, empty_labels);
		create_survey_submit_button(737, 1080 - 1025 + 30, [this]()
		{
			for (int i = 0; i < this->children.size(); ++i)
			{
				if (this->children[i]->name == "browserok.png")
					this->children[i]->is_visible = false;
			}

			//auto new_message = make_unique<InfoText>(743 - 10, 1080 - 1020 + this->offset, 743 - 10, 1080 - 1020 + this->offset, "invisible.png", "Your vote has been submitted.", BROWSER);
			auto new_message = make_unique<ScreenElement>(737, 1080 - 1025 + this->offset + 30, 737 + 152, 1080 - 1025 + 9 + this->offset + 30, "votehasbeensubmitted.png", BROWSER);
			new_message->never_focus = true;
			this->children.push_back(new_message.get());
			this->parent->to_be_added.push_back(std::move(new_message));
		}, []() {});

		auto username_field = make_unique<TextField>(297, 1080 - 1075, 623, 1080 - 1075 + 30, "textfield.png", parent, BROWSER);
		auto password_field = make_unique<TextField>(297, 1080 - 1129, 623, 1080 - 1129 + 30, "textfield.png", parent, BROWSER);
		children.push_back(username_field.get());
		children.push_back(password_field.get());
		parent->to_be_added.push_back(std::move(username_field));
		parent->to_be_added.push_back(std::move(password_field));
	}

	if (website_name == "helphome")
	{
		auto button = make_unique<Button>(693, 1080 - 988, 1360, 1080 - 911, "chatbutton.png", BROWSER, [this]() { this->change_website("helpcenter"); }, []() {}, "", "genlight.png", "gendark.png");
		children.push_back(button.get());
		parent->to_be_added.push_back(std::move(button));
	}

	if (website_name == "helpcenter")
	{
		parent->reached_help_center = true;
		auto help_center = make_unique<HelpCenter>(500, 200, 850 + 162, 950, "helpwindow.png", parent, application); //(0.0, 56.0, 1900.0, 1000.0, "white.png", parent, application);
		children.push_back(help_center.get());
		children.push_back(help_center->button_ptr);
		children.push_back(help_center->text_field_ptr);
		//parent->to_be_added.insert(parent->to_be_added.end() - 2, std::move(help_center));
		/*for (int i = 0; i < parent->screen_elements.size(); ++i)
		{
			if (parent->screen_elements[i].get() == this)
				parent->screen_elements.insert(parent->screen_elements.begin() + i + 1, std::move(help_center));
		}*/
		parent->to_be_added.push_back(std::move(help_center)); //browser_child controls get added in reverse order right now
	}

	if (website_name == "survey")
	{
		int start_y = 750;
		for (int i = 0; i < survey_questions1.size(); ++i)
			start_y = create_radio_button_group(200, start_y - 50, survey_questions1[i].size() - 1, false, survey_questions1[i]) - 100;

		sum_of_heights -= (start_y - 100);
		scroll_bar->is_visible = true;
		scroll_bar->y2 = 1024.0;
		float height = (1080.0 / ((float)sum_of_heights)) * 948.0;
		scroll_bar->y1 = scroll_bar->y2 - height;
		create_survey_submit_button(glutGet(GLUT_WINDOW_WIDTH) - 400, glutGet(GLUT_WINDOW_HEIGHT) - sum_of_heights + 100, [this]() {this->change_website("survey2");}, [this]() {this->parent->start_application(INFO_BOX, "Error: Left questions blank.");});
	}

	if (website_name == "survey2")
	{
		auto title_label = make_unique<InfoText>(200 - 10, 850, 200 - 10, 850, "invisible.png", "On a scale of 1 to 10 (left to right), how well do the following words describe the trial product you tested?", BROWSER);
		children.push_back(title_label.get());
		int start_y = 750;
		for (int i = 0; i < survey_questions2.size(); ++i)
			start_y = create_radio_button_group(200, start_y - 50, 10, true, survey_questions2[i]);

		create_survey_submit_button(700, 100, [this]() {this->change_website("survey3");}, [this]() {this->parent->start_application(INFO_BOX, "Error: Left questions blank.");});
		parent->to_be_added.push_back(std::move(title_label));
	}

	if (website_name == "survey3")
	{
		auto title_label = make_unique<InfoText>(200 - 10, 850, 200 - 10, 850, "invisible.png", "Thank you for completing the survey.", BROWSER);
		children.push_back(title_label.get());
		parent->to_be_added.push_back(std::move(title_label));
	}

	if (website_name == "contestdesc")
	{
		auto button = make_unique<Button>(750.0, 100.0, 950.0, 130.0, "ok.png", BROWSER,
			[this]()
		{
			parent->start_application(FILES, "", true);
			auto files = make_unique<Files>(770.0, 404.0, 1150.0, 744.0, "white.png", parent, INFO_BOX);
			auto files_ptr = files.get();
			auto button = make_unique<Button>(955.0, 409.0, 1145.0, 459.0, "ok.png", INFO_BOX,
				[this, files_ptr]()
			{
				files_ptr->upload_image_file();
				this->parent->close_application(INFO_BOX);
				this->change_website("thankyouforuploading");
				this->parent->entered_contest = true;
			}, [](){});
			parent->to_be_added.insert(parent->to_be_added.end() - 1, std::move(files));
			parent->to_be_added.push_back(std::move(button));
		}, [](){});

		children.push_back(button.get());
		parent->to_be_added.push_back(std::move(button));
	}

	if (website_name == "centerracomtrial")
	{
		auto text_field = make_unique<TextField>(650, -130, 850, -100, "textfield.png", parent, application, [](){});
		auto text_field_ptr = text_field.get();
		auto button = make_unique<Button>(900, -130, 1000, -100, "ok.png", application,
			[this, text_field_ptr]()
		{
			if (text_field_ptr->text == "xj5555585@nenda.en.os.esn")
				this->parent->entered_right_email_address = true;

			this->change_website("centerracomthankyou");
		}, [](){});

		children.push_back(text_field_ptr);
		children.push_back(button.get());
		parent->to_be_added.push_back(std::move(text_field));
		parent->to_be_added.push_back(std::move(button));
	}

	if (website_name == "hammercounters1")
		parent->start_playing_escape_sound();

	if (website_name == "reddit" || website_name == "tablet" || website_name == "conversation" || website_name == "tomthread")
	{
		do_layout(website_name, 207, 1000 - 200);
		auto name_field = make_unique<TextField>(1426, 820 - 80, 1619, 853 - 80, "textfield.png", parent, BROWSER);
		auto password_field = make_unique<TextField>(1426, 763 - 80, 1619, 796 - 80, "textfield.png", parent, BROWSER);
		password_field->password = true;
		auto nameptr = name_field.get();
		auto passptr = password_field.get();
		auto error_message = make_unique<ExpandableImage>(1702 - 216, 698 - 80 + 11, 1702, 698 - 80 + 11, "pleaseenter.png", BROWSER, 11);
		auto errptr = error_message.get();
		children.push_back(errptr);
		parent->to_be_added.push_back(std::move(error_message));
		auto login_button = make_unique<Button>(1314, 698 - 80, 1425, 733 - 80, "ok.png", BROWSER, [this, errptr, nameptr, passptr]() 
		{
			if (nameptr->text == "" || passptr->text == "")
			{
				if (errptr->y2 - errptr->y1 == 0.0)
					errptr->toggle();
			}

			else
			{
				this->change_website("vote");
				if (errptr->y2 - errptr->y1 > 0.0)
					errptr->toggle();
			}
		}, []() {});
		children.push_back(name_field.get());
		children.push_back(password_field.get());
		children.push_back(login_button.get());
		parent->to_be_added.push_back(std::move(name_field));
		parent->to_be_added.push_back(std::move(password_field));
		parent->to_be_added.push_back(std::move(login_button));
		auto header = make_unique<ScreenElement>(170, 1000 - 155, 1749, 1000, "sleepynews.png", BROWSER);
		header->never_focus = true;
		children.push_back(header.get());
		parent->to_be_added.push_back(std::move(header));
		auto username_box = make_unique<ScreenElement>(1296, 800 - 194, 1296 + 416, 800, "usernamebox.png", BROWSER);
		username_box->never_focus = true;
		children.push_back(username_box.get());
		parent->to_be_added.push_back(std::move(username_box));
	}

	if (website_name == "contestresults")
	{
		auto static_canvas = make_unique<StaticCanvas>(100, 800 - parent->uploaded_image.ysize, 100 + parent->uploaded_image.xsize, 800, "", BROWSER, parent->uploaded_image.image);
		static_canvas->never_focus = true;
		children.push_back(static_canvas.get());
		parent->to_be_added.push_back(std::move(static_canvas));
	}

	if (website_name == "game")
	{
		auto game = make_unique<BubbleGame>(0.0, 56.0, 1920.0, 1000.0, "black.png", parent, BROWSER);
		children.push_back(game.get());
		parent->to_be_added.push_back(std::move(game));
	}

	if (website_name == "weirdstuff")
	{
		auto mmo = make_unique<MMO>(0.0, 56.0, 1920.0, 1000.0, "black.png", "mmo.txt", parent, BROWSER);
		children.push_back(mmo.get());
		parent->to_be_added.push_back(std::move(mmo));
	}

	if (website_name == "recoverpassword")
	{
		auto password1 = make_unique<TextField>(500, 800, 800, 900, "textfield.png", parent, BROWSER);
		auto password2 = make_unique<TextField>(500, 600, 800, 700, "textfield.png", parent, BROWSER);
		password1->password = password2->password = true;
		auto ptr1 = password1.get();
		auto ptr2 = password2.get();
		auto button = make_unique<Button>(500, 400, 600, 500, "ok.png", BROWSER, [this, ptr1, ptr2]()
		{
			if (ptr1->text == ptr2->text)
				parent->mmo_password = ptr1->text;
		}, []() {});
		children.push_back(password1.get());
		children.push_back(password2.get());
		children.push_back(button.get());
		parent->to_be_added.push_back(std::move(password1));
		parent->to_be_added.push_back(std::move(password2));
		parent->to_be_added.push_back(std::move(button));
	}

	if (website_name == "searchresults")
		do_layout("searchresults", 20, 900);

	for (int i = 0; i < children.size(); ++i)
		children[i]->browser_child = true;
}

int ScrollPanel::create_radio_button_group(int start_x, int start_y, int number, bool horizontal, vector<string> labels)
{
	if (!horizontal)
	{
		auto label = make_unique<InfoText>(start_x, start_y + 60, start_x, start_y + 40, "white.png", labels[0], BROWSER);
		children.push_back(label.get());
		parent->to_be_added.push_back(std::move(label));
	}

	int x_offset = 0, y_offset = 0;
	if (horizontal)
		x_offset += 120;

	auto master_button = make_unique<RadioButton>(start_x + x_offset, start_y, start_x + 25 + x_offset, start_y + 25, "radiobutton.png", parent, BROWSER);
	auto master_label = make_unique<InfoText>(start_x + 25, start_y + 5, start_x + 25, start_y + 5, "white.png", labels[1], BROWSER);
	children.push_back(master_button.get());
	children.push_back(master_label.get());
	number--;
	int change = 30;
	int* to_change;
	if (horizontal)
		to_change = &x_offset;

	else
	{
		to_change = &y_offset;
		change *= -1;
	}

	for (int i = 0; i < number - 1; ++i)
	{
		*to_change += change;
		auto child_button = make_unique<RadioButton>(start_x + x_offset, start_y + y_offset, start_x + x_offset + 25, start_y + y_offset + 25, "radiobutton.png", parent, BROWSER, master_button.get());
		master_button->children.push_back(child_button.get());
		children.push_back(child_button.get());
		parent->to_be_added.push_back(std::move(child_button));
		if (!horizontal)
		{
			auto label = make_unique<InfoText>(start_x + x_offset + 25, start_y + y_offset + 5, start_x + x_offset + 75, start_y + y_offset + 5, "invisible.png", labels[i + 2], BROWSER);
			children.push_back(label.get());
			parent->to_be_added.push_back(std::move(label));
		}
	}

	parent->to_be_added.push_back(std::move(master_button));
	parent->to_be_added.push_back(std::move(master_label));
	return start_y + y_offset;
}

void ScrollPanel::create_survey_submit_button(int x, int y, function<void()> on_success, function<void()> on_failure)
{
	auto button = make_unique<Button>(x, y, x + 150, y + 40, "browserok.png", BROWSER,
		[this, on_success, on_failure]()
	{
		for (int i = 0; i < this->children.size(); ++i)
		{
			auto f = this->children[i];
			RadioButton* radio_button = dynamic_cast<RadioButton*>(this->children[i]);
			if (radio_button != NULL)
			{
				if (!radio_button->has_selection())
				{
					on_failure();
					return;
				}
			}
		}

		on_success();
	}, [](){}, "", "browserheld.png", "browserpressed.png");
	children.push_back(button.get());
	parent->to_be_added.push_back(std::move(button));
}

void ScrollPanel::remove_children()
{
	for (int i = 0; i < parent->screen_elements.size(); ++i)
	{
		for (int j = 0; j < children.size(); ++j)
		{
			if (children[j] == parent->screen_elements[i].get())
				parent->screen_elements[i]->marked_for_deletion = true;
		}
	}

	children.clear();
}

void ScrollPanel::start_loading_site(string website_name, bool force_through)
{
	if (!whitelist_check(website_name, force_through))
	{
		change_website("error", true, true);
		return;
	}

	currently_loading_site = true;
	site_to_be_loaded = website_name;
	loading_time = rand() % 1000;
	if (loading_time == 0)
		loading_time = 60;

	time_started_loading = glutGet(GLUT_ELAPSED_TIME);
}

void ScrollPanel::draw(TextureManager* texture_manager)
{
    glEnable(GL_STENCIL_TEST);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    glStencilFunc(GL_NEVER, 1, 0xFF);
    glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
    glDisable(GL_TEXTURE_2D);
    glStencilMask(0xFF);
    glColor4f(1.0, 0.0, 0.0, 1.0);
	float lower_y = 56.0;
	float upper_y = 1080.0;
	if (y1 > lower_y)
	{
		lower_y = y1;
		upper_y = y2;
	}

    glRectf(0.0, scaley(lower_y), scalex(1920.0), scaley(upper_y));
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glEnable(GL_TEXTURE_2D);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    float old_offset = offset;
	if (offset > sum_of_heights - 1080.0)
	{
		offset = sum_of_heights - 1080.0;
		move_links(offset - old_offset);
	}

	float current_y = this->y2 + offset;
	bool increase_last_height = false;
	int real_sum_of_heights = 0;
	for (int i = 0; i < textures.size(); ++i)
		real_sum_of_heights += texture_manager->get_height(textures[i]);

	if (real_sum_of_heights < sum_of_heights)
		increase_last_height = true;

	/*for_each(textures.begin(), textures.end(), [&](string& texture)
	{
		int height = texture_manager->get_height(texture);
		texture_manager->change_texture(texture);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0, 1.0); glVertex2f(scalex(this->x1), scaley(current_y - height));
			glTexCoord2f(0.0, 0.0); glVertex2f(scalex(this->x1), scaley(current_y));
			glTexCoord2f(1.0, 0.0); glVertex2f(scalex(this->x2), scaley(current_y));
			glTexCoord2f(1.0, 1.0); glVertex2f(scalex(this->x2), scaley(current_y - height));
		glEnd();
		current_y -= height;
	});*/

	for (int i = 0; i < textures.size(); ++i)
	{
		int height = texture_manager->get_height(textures[i]);
		if (i == textures.size() - 1 && increase_last_height)
			height += sum_of_heights - real_sum_of_heights;
		texture_manager->change_texture(textures[i]);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0, 1.0); glVertex2f(scalex(this->x1), scaley(current_y - height));
			glTexCoord2f(0.0, 0.0); glVertex2f(scalex(this->x1), scaley(current_y));
			glTexCoord2f(1.0, 0.0); glVertex2f(scalex(this->x2), scaley(current_y));
			glTexCoord2f(1.0, 1.0); glVertex2f(scalex(this->x2), scaley(current_y - height));
		glEnd();
		current_y -= height;
	}

	if (textures[0] == "nendablank.png") //draw emails
	{
		ScreenElement email_link(73.0 + 15, 1080 - 286, 73 + 15, 1080 - 286, "");
		for (int i = 0; i < parent->emails.size(); ++i)
		{
			string email_name = parent->emails[i];
			if (parent->read_email[email_name])
				email_link.name = email_name.substr(0, email_name.length() - 8) + "regular.png";

			else email_link.name = email_name;
			float email_height = texture_manager->get_height(email_name);
			email_link.y2 = email_link.y1 + email_height;
			email_link.x2 = email_link.x1 + texture_manager->get_width(email_name);
			ScreenElement dot(73, floor(email_height / 2.0) - 2 + email_link.y1, 79, floor(email_height / 2.0) + 4 + email_link.y1, "blackdot.png");
			dot.draw(texture_manager);
			email_link.draw(texture_manager);
			email_link.y1 -= 35;
			email_link.y2 -= 35;
		}
	}

	glStencilMask(~0);
	glClear(GL_STENCIL_BUFFER_BIT);
	glDisable(GL_STENCIL_TEST);
	if (currently_loading_site)
	{
		string message = "Transferring data from " + site_to_be_loaded + "...";
		vector<float> widths = draw_string(texture_manager, 32, message, 0, 0, false);
		float width = widths[widths.size() - 1];
		ScreenElement beige = ScreenElement(0.0, 56.0, width, 70.0, "beigeold.png");
		beige.draw(texture_manager);
		draw_string(texture_manager, 32, message, 0.0, 57.0);
	}

	if (scroll_locked)
	{
		parent->draw_arrow_thing = true;
		parent->scroll_lock_x = scroll_lock_x;
		parent->scroll_lock_y = scroll_lock_y;
	}
}

void ScrollPanel::press_key(unsigned char key)
{
    float old_offset = offset;
    switch (key)
    {
        case KEY_UP:
            if (going_up && timebase != -1)
                return;
            
            going_up = true;
            offset -= scroll_speed;
            if (offset < 0)
                offset = 0;
            scroll_bar->move((948.0 * scroll_speed) / sum_of_heights);
            break;
            
        case KEY_DOWN:
            if (!going_up && timebase != -1)
                return;
            
            going_up = false;
            offset += scroll_speed;
            scroll_bar->move((948.0 * -scroll_speed) / sum_of_heights);
            break;
            
        default:
            return;
    }
    
    timebase = glutGet(GLUT_ELAPSED_TIME);
    move_links(offset - old_offset);
    mouse_over(prev_mouse_x, prev_mouse_y);
}

void ScrollPanel::release_key(unsigned char key)
{
    if ( (key == KEY_UP && going_up) || (key == KEY_DOWN && !going_up) )
        timebase = -1;
}

void ScrollPanel::animate()
{
	animation_function();
	if (currently_loading_site)
	{
		int cur_time = glutGet(GLUT_ELAPSED_TIME);
		if (cur_time - time_started_loading > loading_time)
		{
			currently_loading_site = false;
			change_website(site_to_be_loaded);
			parent->process_control_changes();
		}
	}

    if (timebase == -1)
        return;
    
    if (glutGet(GLUT_ELAPSED_TIME) - timebase < 500 && !scroll_locked)
        return;
    
    float old_offset = offset;
    if (going_up)
    {
        offset -= scroll_speed;
        if (offset < 0)
            offset = 0;
        
        scroll_bar->move((948.0 * scroll_speed) / sum_of_heights);
    }
    
    else
    {
        offset += scroll_speed;
        scroll_bar->move((948.0 * -scroll_speed) / sum_of_heights);
    }
    
    move_links(offset - old_offset);
    mouse_over(prev_mouse_x, prev_mouse_y);
}

//TODO: replace most references to this e.g. from buttons and links with start_loading_site()
void ScrollPanel::change_website(std::string website_name, bool new_branch, bool force_through, string append_text)
{
	if (!whitelist_check(website_name, force_through))
		change_website("error", true, true);

	animation_function = []() {};
	extend = false;
	if (website_name == "e2")
		parent->forcing_e2 = true;

	if (website_name == "vote")
	{
		parent->greyed_out = true;
		parent->start_application(INFO_BOX, "You must be logged in to perform that function.", false, false, 0, this);
		return;
	}
	
	if (website_name == "paint")
	{
		parent->start_application(INFO_BOX, "", false, true);
		return;
	}

	if (website_name == "download_trial")
	{
		parent->start_application(INFO_BOX, "", false, true, 1);
		return;
	}

	if (website_name == "decrypt")
	{
		parent->start_application(INFO_BOX, "", false, true, 2);
		return;
	}

	if (website_name == "mmo")
	{
		parent->start_application(INFO_BOX, "", false, true, 3);
		return;
	}
	
	if (website_name == prev_website)
		return;

	if (required_target != "")
	{
		website_name = required_target;
		required_target = "";
	}

	if (websites.find(website_name) == websites.end())
	{
		change_website("error");
		return;
	}

	prev_website = website_name;
	//TODO ORIGINAL LOCATION OF MARKING VISITED_SITE
	text_field_ptr->reset();
	text_field_ptr->text = website_name;
	if (parent->read_email["rosecolored1bold.png"] && (website_name == "search" || website_name == "reddit" || website_name == "useful")) //TODO doesn't check enough pages
		parent->visited_elsewhere_after_rosecolored1 = true;
	
	if (new_branch)
	{
		if (!history.empty())
			history.erase(history.begin() + history_index + 1, history.end());

		history.push_back(website_name);
		history_index++;
	}
    
	offset = 0;
	textures = websites[website_name];
    sum_of_heights = 0; //TODO used to be 56
	if (/*textures.size() == 1*/ parent->texture_manager->get_height(textures[0]) <= 1080.0)
	{
		sum_of_heights = 0;
		scroll_bar->is_visible = false;
	}

	else
		scroll_bar->is_visible = true;

    for (int i = 0; i < textures.size(); ++i)
        sum_of_heights += parent->texture_manager->get_height(textures[i]);
    
    scroll_bar->y2 = 1024.0;
    float height = (1080.0 / ((float) sum_of_heights)) * 948.0;
    scroll_bar->y1 = scroll_bar->y2 - height;
	links.clear();
	for_each(textures.begin(), textures.end(), [this](string& texture)
	{
		string file_name;
		if (alternative_links.count(texture) > 0)
			file_name = alternative_links[texture];

		else file_name = split(texture, '.')[0] + ".link";
		ifstream file_stream(file_name);
		string line;
		while (getline(file_stream, line))
		{
			vector<string> components = split(line, ' ');
			links.push_back(Link(Rect(atof(components[0].c_str()), atof(components[1].c_str()), atof(components[2].c_str()), atof(components[3].c_str())), components[4]));
		}
	});

	if (website_name == "rosecolored4" && !parent->visited_site[website_name])
		required_target = "search";

	/*if (website_name == "ain" && parent->reached_help_center)
		links[0].target = "psych";*/

	if (website_name == "psych")
		parent->reached_psych_article = true;

	if (textures[0] == "nendablank.png")
	{
		if (parent->visited_site["search"] && parent->visited_site["reddit"] && parent->visited_site["tablet"] && parent->visited_site["useful"] && parent->visited_site["nenda"] &&
			parent->visited_site["rfh1"] && parent->visited_site["rfh2"] && parent->visited_site["forumemail1"] && parent->visited_site["forumemail2"] && parent->visited_site["deliverfailure"] &&
			parent->visited_site["ain"] && parent->visited_site["e1"] && parent->visited_site["conversation"] && parent->visited_site["tomthread"] && parent->no_rosecolored1)
		{
			parent->emails.insert(parent->emails.begin(), "rosecolored1bold.png");
			parent->no_rosecolored1 = false;
		}

		if (parent->visited_elsewhere_after_rosecolored1 && parent->no_rosecolored2)
		{
			parent->emails.insert(parent->emails.begin(), "rosecolored2bold.png");
			parent->no_rosecolored2 = false;
		}

		if (parent->looked_for_spider && parent->no_rosecolored3)
		{
			parent->emails.insert(parent->emails.begin(), "rosecolored3bold.png");
			parent->no_rosecolored3 = false;
		}

		if (parent->tried_to_view_encrypted_folder && parent->no_rosecolored4)
		{
			parent->emails.insert(parent->emails.begin(), "rosecolored4bold.png");
			parent->no_rosecolored4 = false;
		}

		if (parent->missed_spider && parent->no_rosecolored_missed_spider)
		{
			parent->emails.insert(parent->emails.begin(), "rosecoloredmissedspiderbold.png");
			parent->no_rosecolored_missed_spider = false;
		}

		if (parent->reached_psych_article && parent->no_rosecolored5)
		{
			parent->emails.insert(parent->emails.begin(), "rosecolored5bold.png");
			parent->no_rosecolored5 = false;
		}

		if (parent->entered_right_email_address && parent->no_centerracom_trial_link)
		{
			parent->emails.insert(parent->emails.begin(), "triallinkbold.png");
			parent->no_centerracom_trial_link = false;
		}

		if (parent->ran_trial && parent->no_centerracom_survey_link)
		{
			parent->emails.insert(parent->emails.begin(), "surveylinkbold.png");
			parent->no_centerracom_survey_link = false;
		}

		float y1 = 1080 - 286;
		for (int i = 0; i < parent->emails.size(); ++i)
		{ //link emails
			links.push_back(Link(Rect(73.0 + 15, y1, 73 + 15 + parent->texture_manager->get_width(parent->emails[i]), y1 + parent->texture_manager->get_height(parent->emails[i])), parent->emails[i].substr(0, parent->emails[i].length() - 8)));
			y1 -= 35.0;
		}
	}
    
	parent->visited_site[website_name] = true;
	remove_children();
	add_children(website_name);
	glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
	text_field_ptr->text += append_text;
}

void ScrollPanel::tick_scrollbar(int ticks)
{
    float old_offset = offset;
    offset -= (float)(sum_of_heights * ticks) / 948.0;
    if (offset < 0.0)
        offset = 0.0;
    
    move_links(offset - old_offset);
    mouse_over(prev_mouse_x, prev_mouse_y);
}

void ScrollPanel::move_links(float delta)
{
	for_each(links.begin(), links.end(), [delta](Link& link)
	{
		link.rect.y1 += delta;
		link.rect.y2 += delta;
	});

	for (int i = 0; i < children.size(); ++i)
	{
		children[i]->y1 += delta;
		children[i]->y2 += delta;
	}
}

void ScrollPanel::mouse_over(int x, int y)
{
	prev_mouse_x = x;
	prev_mouse_y = y;
	for (int i = 0; i < links.size(); ++i)
	{
		Link link = links[i];
		if (link.rect.x1 <= x && x <= link.rect.x2 && link.rect.y1 <= y && y <= link.rect.y2)
		{
			glutSetCursor(GLUT_CURSOR_WAIT);
			parent->someone_set_cursor = true;
			return;
		}
	}

	glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
}

void ScrollPanel::mouse_moved(int x, int y)
{
	if (!scroll_locked)
		return;

	if (y > scroll_lock_y + 10)
	{
		release_key(KEY_DOWN);
		scroll_speed = (y - scroll_lock_y) / 10.0;
		press_key(KEY_UP);
	}

	else if (y < scroll_lock_y - 10)
	{
		release_key(KEY_UP);
		scroll_speed = (scroll_lock_y - y) / 10.0;
		press_key(KEY_DOWN);
	}

	else
	{
		release_key(KEY_UP);
		release_key(KEY_DOWN);
		scroll_speed = 40.0;
	}
}

void ScrollPanel::turn_off_scroll_lock()
{
	if (scroll_locked)
	{
		scroll_locked = false;
		scroll_speed = 40.0;
		release_key(KEY_UP);
		release_key(KEY_DOWN);
	}
}

void ScrollPanel::take_focus()
{
	turn_off_scroll_lock();
}

void ScrollPanel::mouse_clicked(int button, int state, int x, int y)
{
	if (button == 1 && state == GLUT_UP)
		return;

	if (button == 0 && state == GLUT_UP)
		return;
	
	if (button == 1 && state == GLUT_DOWN)
	{
		if (!scroll_locked)
		{
			scroll_locked = true;
			scroll_lock_x = x;
			scroll_lock_y = y;
		}

		else
			turn_off_scroll_lock();

		return; //TODO should middle button clicking on a link take you to the site?
	}

	turn_off_scroll_lock();
	if (button == 3)
	{
		this->press_key(KEY_UP);
		this->release_key(KEY_UP);
		return;
	}

	if (button == 4)
	{
		this->press_key(KEY_DOWN);
		this->release_key(KEY_DOWN);
		return;
	}
	
	for (int i = 0; i < links.size(); ++i)
    {
        Link link = links[i];
		if (link.rect.x1 <= x && x <= link.rect.x2 && link.rect.y1 <= y && y <= link.rect.y2)
		{
			if (link.target.substr(0, 8) == "sequence")
			{
				if (sequences[link.target].empty())
				{
					links[i].target = "error";
					link.target = "error";
				}

				else
				{
					string new_target = sequences[link.target][sequences[link.target].size() - 1];
					sequences[link.target].pop_back();
					links[i].target = new_target;
					link.target = new_target;
				}
			}

			if (textures[0] == "nendablank.png")
			{
				auto it = parent->read_email.find(link.target + "bold.png");
				if (it != parent->read_email.end())
					parent->read_email[link.target + "bold.png"] = true;
			}

			change_website(link.target, true, true);
		}
    }
}

void ScrollPanel::mouse_off()
{
	//I don't remember why I had to do this
	//And I don't remember why I decided to comment it out either
    //glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
}

void ScrollPanel::go_back()
{
    if (history_index == 0)
        return;
    
    history_index--;
    change_website(history[history_index], false);
}

void ScrollPanel::go_forward()
{
    if (history_index < (history.size() - 1))
    {
        history_index++;
        change_website(history[history_index], false);
    }
}