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
#include "RotatingBox.h"
#include "SlidingTile.h"
#include "LoadingBar.h"
#include "ShoppingCenter.h"
#include "ProgressBar.h"
#include "Video.h"
#include "ExpandableHeader.h"
#include "CachedText.h"
#include "BlankEssay.h"
#include "StaticText.h"
#include "Animation.h"
#include "ReflectedObject.h"
#include "RectUtilities.h"
#include "LoadingIndicator.h"
#include "keymappings.h"

using namespace std;
void save_game(string filename);

//TODO: Scrolling slows down when moving the mouse
//TODO should not be able to upclick to activate links
//TODO when making glass, clicking on very top pixel is error?  Because right now y clicks are indexed 1 to 1080
//TODO need to add never_focus to more browser children, e.g. click a radio button on a page and up and down arrow keys don't scroll anymore
//TODO press go button on superuseful, animation stops

char random_char()
{
	int num = rand() % 36;
	if (num <= 9)
		return (char)(num + 48);

	else return (char)(num + 87);
}

char random_num()
{
	int num = rand() % 10;
	return (char)(num + 48);
}

int num_digits(string str)
{
	int num = 0;
	for (int i = 0; i < str.size(); ++i)
	{
		if (isdigit(str[i]))
			num++;
	}

	return num;
}

ScrollPanel::ScrollPanel(float _x1, float _y1, float _x2, float _y2, Computer* _parent, Application _application, bool no_extras) : ScreenElement(_x1, _y1, _x2, _y2, "", _application), parent(_parent), offset(0)
{
	will_force_through = false;
	white_cover_alpha = CycleNumber(0.0, 1.0, 0.03);
	white_cover_apex = []() {};
	is_mouse_over = false;
	underlinex1 = underlinex2 = underliney = -1;
	int toolbar_base = 1080 - WINDOW_TITLE_HEIGHT - BROWSER_TOOLBAR_HEIGHT;
	int window_title_base = 1080 - WINDOW_TITLE_HEIGHT;
	name = "white.png";
	num_break_points = 2;
	big_glass_height = 1200.0;
	do_whitelist_check = false;
	surrender_focus_on_up_click = false;
	animation_function = []() {};
	if (!no_extras)
		parent->to_be_added.push_back(make_unique<ScreenElement>(0.0, toolbar_base, 1920.0, window_title_base, "beige.png", application));
	
	auto text_field = make_unique<TextField>(20, toolbar_base + 4, 700, toolbar_base + 34, "systemtextfield.png", parent, application);
	text_field_ptr = text_field.get();
	parent->to_be_added.push_back(make_unique<ScreenElement>(720 - 3, toolbar_base + 1, 750 + 3, toolbar_base + 37, "invisible.png", application));
	auto button = make_unique<Button>(1661, toolbar_base + 9, 1681, toolbar_base + 29, "go.png", application,
		[this]()
	{
		if (this->parent->everything_stuck)
			this->parent->url_bucket++;

		string id_name = "error";
		for (auto &i : website_names)
		{
			if (i.second == this->text_field_ptr->text)
			{
				id_name = i.first;
				break;
			}
		}

		this->trying_to_load = this->text_field_ptr->text;
		if (id_name == "error" || !whitelist_check(id_name, false))
			this->error_display = this->text_field_ptr->text;

		this->start_loading_site(id_name, false);
	}, [](){}, "", "golight.png", "godark.png", "", false, true);
	auto button_ptr = button.get();
	text_field_ptr->on_enter_function = [button_ptr]() { button_ptr->release_function(); };
	text_field_ptr->draw_first_cap = false;
	parent->to_be_added.push_back(std::move(text_field));
	auto altcap = make_unique<ScreenElement>(text_field_ptr->x1 - 24, text_field_ptr->y1, text_field_ptr->x1 + 2, text_field_ptr->y2, "altcap.png", application);
	altcap->is_visible = false;
	text_field_ptr->altcap = altcap.get();
	/*auto altcap_button = make_unique<Button>(text_field_ptr->x1 - 24, text_field_ptr->y1, text_field_ptr->x1 + 2, text_field_ptr->y2, "altcapbutton.png", BROWSER, [this]()
	{
		auto not_secure = make_unique<ScreenElement>(this->text_field_ptr->x1 - 88, this->text_field_ptr->y1 + 5 - 64, -1, -1, "notsecure.png", BROWSER);
		not_secure->transient = true;
		this->parent->to_be_added.push_back(std::move(not_secure));
	}, 
	[](){}, "", "altcapbuttonhover.png", "altcapbuttonhover.png");*/
	auto altcap_button = make_unique<Button>(text_field_ptr->x1 - 24, text_field_ptr->y1, text_field_ptr->x1 + 2, text_field_ptr->y2, "altcapbutton.png", application, [](){}, []() {}, "", "altcapbuttonhover.png", "altcapbuttonhover.png");
	auto altcap_ptr = altcap_button.get();
	altcap_button->is_visible = false;
	altcap_button->release_function = [this, altcap_ptr]()
	{
		auto not_secure = make_unique<ScreenElement>(this->text_field_ptr->x1 - 88, this->text_field_ptr->y1 + 5 - 64, -1, -1, "notsecure.png", application);
		not_secure->transient = true;
		this->parent->to_be_added.push_back(std::move(not_secure));
		altcap_ptr->name = "altcapbuttonhover.png";
	};
	altcap_button->mouse_over_function = [this]() {this->text_field_ptr->change_names(); this->text_field_ptr->mouse_on = false;};
	auto altcap_button_ptr = altcap_button.get();
	altcap_button->mouse_off_function = [this, altcap_button_ptr]() {if (!altcap_button_ptr->is_mouse_over) return; this->text_field_ptr->reset_names();};
	parent->to_be_added.push_back(std::move(altcap));
	parent->to_be_added.push_back(std::move(altcap_button));
	parent->to_be_added.push_back(std::move(button));
	
	if (!no_extras)
	{
		parent->to_be_added.push_back(make_unique<Button>(665.0, 1009.0, 695.0, 1029.0, "history.png", application, []() {},
			[this]()
		{
			for (int i = history.size() - 1; i >= 0 && history.size() - i < 6; --i)
			{
				auto history_button = make_unique<Button>(100.0, text_field_ptr->y1 - (75.0 * (history.size() - i)), 700.0, text_field_ptr->y1 - (75.0 * (history.size() - i - 1)), "white.png", application,
					[this, i]()
				{
					this->change_website(this->history[i].first);
				}, []() {}, history[i].first);
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
		altcap_ptr->is_visible = false;
		text_field_ptr->altcap->is_visible = false;
	}

	website_names["search"] = "search.tn.co.jtz";
	website_names["ain"] = "ain.qp.me.ttb";
	website_names["e1"] = "ain.qp.me.ttb/article?id=6722891910";
	website_names["advertising"] = "ain.qp.me.ttb/advertising";
	website_names["purchase"] = "ain.qp.me.ttb/purchase_premium";
	website_names["forumhome"] = "hammercounters.tj.mm.iop";
	website_names["forumthreads"] = "hammercounters.tj.mm.iop/forum?id=576";
	website_names["purchasecomplete"] = "ain.qp.me.ttb/purchase_complete";
	website_names["reddit"] = "sleepy.oo.oo.zzz";
	website_names["conversation"] = "sleepy.oo.oo.zzz/thread?id=67a8n";
	website_names["tablet"] = "sleepy.oo.oo.zzz/thread?id=4ba8p";
	website_names["tomthread"] = "sleepy.oo.oo.zzz/thread?id=f55k0";
	website_names["nenda"] = "nenda.en.os.esn";
	website_names["spam"] = "nenda.en.os.esn/spam";
	website_names["trash"] = "nenda.en.os.esn/trash";
	website_names["about"] = "nenda.en.os.esn/about";
	website_names["rfh1"] = "nenda.en.os.esn/email?id=6711851092";
	website_names["rfh2"] = "nenda.en.os.esn/email?id=7001890841";
	website_names["rfh5"] = "nenda.en.os.esn/email?id=0953867661";
	website_names["forumemail1"] = "nenda.en.os.esn/email?id=0451181185";
	website_names["forumemail2"] = "nenda.en.os.esn/email?id=6321810907";
	website_names["deliverfailure"] = "nenda.en.os.esn/email?id=7874925178";
	website_names["rosecolored1"] = "nenda.en.os.esn/email?id=3176823154";
	website_names["rosecolored2"] = "nenda.en.os.esn/email?id=4857718623";
	website_names["rosecoloredmissedspider"] = "nenda.en.os.esn/email?id=6885239000";
	website_names["rosecolored3"] = "nenda.en.os.esn/email?id=7671823411";
	website_names["useful"] = "eniik.th.bi.ora";
	website_names["rosecolored4"] = "nenda.en.os.esn/email?id=0892451870";
	website_names["rfh3"] = "nenda.en.os.esn/email?id=8118758201";
	website_names["searchresults"] = "search.tn.co.jtz/search?q=help";
	website_names["helphome"] = "professionalaffordablehelp.he.lp.now";
	website_names["psychologist"] = "ain.qp.me.ttb/article?id=7143829117";
	website_names["rfh4"] = "nenda.en.os.esn/email?id=4448339921";
	website_names["rosecolored5"] = "nenda.en.os.esn/email?id=9093991434";
	website_names["paint"] = "downloadpaintprograms.co.th.ghz";
	website_names["thankyouforuploading"] = "ent5n.ei.ap.nex/users/~walter/upload_successful";
	website_names["centerracom"] = "centerracom.en.gx.tnq";
	website_names["centerracomtrial"] = "centerracom.en.gx.tnq/trial";
	website_names["centerracomcontact"] = "centerracom.en.gx.tnq/contact";
	website_names["centerracomthankyou"] = "centerracom.en.gx.tnq/thank_you";
	website_names["funcorner"] = "centerracom.en.gx.tnq/fun";
	website_names["triallink"] = "nenda.en.os.esn/email?id=3421386889";
	website_names["download_trial"] = "centerracom.en.gx.tnq/software_trial?download_action=b98458b23d36b181fcff4af0dc8e7816d95b494118eecead2f9ed54c50091ed3779f33114513c8ce1f5e3bc76ca06444";
	if (parent->ran_trial)
	{
		website_names.erase("download_trial");
		website_names["trialexpired"] = "centerracom.en.gx.tnq/software_trial?download_action=b98458b23d36b181fcff4af0dc8e7816d95b494118eecead2f9ed54c50091ed3779f33114513c8ce1f5e3bc76ca06444";
	}

	website_names["survey"] = "centerracom.en.gx.tnq/survey?id=7aj3n11ma89fj32tqk83&page=1";
	website_names["survey2"] = "centerracom.en.gx.tnq/survey?id=7aj3n11ma89fj32tqk83&page=2";
	website_names["survey3"] = "centerracom.en.gx.tnq/survey?id=7aj3n11ma89fj32tqk83&page=3";
	website_names["survey4"] = "centerracom.en.gx.tnq/survey?id=7aj3n11ma89fj32tqk83&page=4";
	website_names["surveylink"] = "nenda.en.os.esn/email?id=2328111950";
	website_names["thread11"] = "hammercounters.tj.mm.iop/thread?id=574822&page=1";
	website_names["thread12"] = "hammercounters.tj.mm.iop/thread?id=574822&page=2";
	website_names["thread21"] = "hammercounters.tj.mm.iop/thread?id=372110&page=1";
	website_names["thread22"] = "hammercounters.tj.mm.iop/thread?id=372110&page=2";
	website_names["thread31"] = "hammercounters.tj.mm.iop/thread?id=898457&page=1";
	website_names["thread32"] = "hammercounters.tj.mm.iop/thread?id=898457&page=2";
	website_names["thread41"] = "hammercounters.tj.mm.iop/thread?id=184321&page=1";
	website_names["thread51"] = "hammercounters.tj.mm.iop/thread?id=908065&page=1";
	website_names["thread52"] = "hammercounters.tj.mm.iop/thread?id=908065&page=2";
	website_names["game"] = "game.xx.xx.xxx";
	website_names["hcthread"] = "sleepy.oo.oo.zzz/thread?id=j88am";
	website_names["imagehost"] = "freeimagehosting.hc.em.gbq/image?id=j7a65nn8";
	website_names["rfh6"] = "nenda.en.os.esn/email?id=5888912710";
	website_names["rosecolored6"] = "nenda.en.os.esn/email?id=9950122844";
	website_names["contestresults"] = "sleepy.oo.oo.zzz/thread?id=k990p";
	website_names["adminnotice"] = "sleepy.oo.oo.zzz/thread?id=m4uua";
	website_names["arcade"] = "paulsarcade.ji.mq.iib";
	website_names["linesexist"] = "paulsarcade.ji.mq.iib/lines";
	website_names["squaresexist"] = "paulsarcade.ji.mq.iib/squares";
	website_names["brandonexist"] = "paulsarcade.ji.mq.iib/brandon";
	website_names["existconquest"] = "paulsarcade.ji.mq.iib/conquest";
	website_names["deliveryexist"] = "paulsarcade.ji.mq.iib/delivery";
	website_names["typingexist"] = "paulsarcade.ji.mq.iib/typing";
	website_names["decryptpage"] = "lorocosoft.ua.mt.qqm/qdecrypt";
	website_names["decrypt"] = "lorocosoft.ua.mt.qqm/download";
	website_names["---"] = "---";
	website_names["contestupload"] = "ent5n.ei.ap.nex/users/~walter/upload";
	website_names["upload"] = "ent5n.ei.ap.nex/users/~walter/contest_upload";
	website_names["walter1"] = "ent5n.ei.ap.nex/users/~walter/welcome";
	website_names["walter2"] = "ent5n.ei.ap.nex/users/~walter/gallery";
	website_names["centerracominvalid"] = "centerracom.en.gx.tnq/email_error";
	website_names["walter3"] = "ent5n.ei.ap.nex/users/~walter/notice";
	website_names["decryptthread"] = "sleepy.oo.oo.zzz/thread?id=iu8la";
	website_names["rfh7"] = "nenda.en.os.esn/email?id=5779333298";

	alternative_links["nendaspam.png"] = "nendablank.link";
	alternative_links["nendatrash.png"] = "nendablank.link";
	alternative_links["nendaabout.png"] = "nendablank.link";
	alternative_links["rfh1.png"] = "nendablank.link";
	alternative_links["rfh2.png"] = "nendablank.link";
	alternative_links["forumemail1.png"] = "nendablank.link";
	alternative_links["forumemail2.png"] = "nendablank.link";
	alternative_links["deliverfailure.png"] = "nendablank.link";
	alternative_links["funcorner.png"] = "centerracomhomepage.link";
	alternative_links["centerracomcontact.png"] = "centerracomhomepage.link";
	alternative_links["centerracompartners.png"] = "centerracomhomepage.link";
	alternative_links["centerracomtrial.png"] = "centerracomhomepage.link";
	alternative_links["centerracomthankyou.png"] = "centerracomhomepage.link";
	alternative_links["centerracominvalid.png"] = "centerracomhomepage.link";
	alternative_links["trialexpired.png"] = "centerracomhomepage.link";
	alternative_links["rosecolored1.png"] = "nendablank.link";
	alternative_links["rosecolored2.png"] = "nendablank.link";
	alternative_links["rosecoloredmissedspider.png"] = "nendablank.link";
	alternative_links["rosecolored3.png"] = "nendablank.link";
	alternative_links["rosecolored4.png"] = "nendablank.link";
	alternative_links["rfh3.png"] = "nendablank.link";
	alternative_links["rfh4.png"] = "nendablank.link";
	alternative_links["ain3.png"] = "ain2.link";
	alternative_links["rfh5.png"] = "nendablank.link";
	alternative_links["rfh6.png"] = "nendablank.link";
	alternative_links["rfh7.png"] = "nendablank.link";
	alternative_links["squaresexist.png"] = "linesexist.link";
	alternative_links["brandonexist.png"] = "linesexist.link";
	alternative_links["deliveryexist.png"] = "linesexist.link";
	alternative_links["typingexist.png"] = "linesexist.link";

	link_names["~a"] = "hammercounters.tj.mm.iop/login";
	link_names["~b"] = "hammercounters.tj.mm.iop/register";
	link_names["~c"] = "hammercounters.tj.mm.iop/rules";
	link_names["~d"] = "hammercounters.tj.mm.iop/report";
	link_names["~e"] = "hammercounters.tj.mm.iop/banned_memebers";
	link_names["~f"] = "hammercounters.tj.mm.iop/apply";
	link_names["~g"] = "hammercounters.tj.mm.iop/history";
	link_names["~h"] = "hammercounters.tj.mm.iop/member_locator";

	special_error_display["sequenceain"] = "ain.qp.me.ttb/article?id=7143829117";

	layout["forumhome"] = { pair<int, string>(0, "header.png"), pair<int, string>(0, "header.png"), pair<int, string>(0, "header.png"), pair<int, string>(0, "header.png"), pair<int, string>(0, "header.png") };
	vector<pair<int, string>> reddit_layout;

	reddit_layout.push_back(pair<int, string>(0, "firstpage.png"));
	reddit_layout.push_back(pair<int, string>(-738, "sleepyfooter.png"));

	vector<pair<int, string>> search_layout;
	for (int i = 0; i < 10; ++i)
		search_layout.push_back(pair<int, string>(0, "result.png"));

	search_layout.push_back(pair<int, string>(-1, "didyoulike.png"));

	vector<pair<int, string>> conversation_layout, tablet_layout, tomthread_layout, contest_layout;
	conversation_layout.push_back(pair<int, string>(0, "conversationtitle.png"));
	tablet_layout.push_back(pair<int, string>(0, "tablettitle.png"));
	tomthread_layout.push_back(pair<int, string>(0, "tomthreadtitle.png"));
	contest_layout.push_back(pair<int, string>(0, "logocontesttitle.png"));
	//for (int i = 0; i < 20; ++i)
	//	conversation_layout.push_back(pair<int, string>(i % 2 == 0 ? 0 : 1, "comment1.png"));
	conversation_layout.push_back(pair<int, string>(0, "thread1.png"));
	for (int i = 0; i < 20; ++i)
		tablet_layout.push_back(pair<int, string>(i % 2 == 0 ? 0 : 1, "comment2.png"));
	for (int i = 0; i < 20; ++i)
		tomthread_layout.push_back(pair<int, string>(i % 2 == 0 ? 0 : 1, "comment3.png"));
	contest_layout.push_back(pair<int, string>(0, "contestcomment.png"));
	conversation_layout.push_back(pair<int, string>(-738, "sleepyfooter.png"));
	tablet_layout.push_back(pair<int, string>(-738, "sleepyfooter.png"));
	tomthread_layout.push_back(pair<int, string>(-738, "sleepyfooter.png"));

	layout["reddit"] = reddit_layout;
	layout["searchresults"] = search_layout;
	layout["conversation"] = conversation_layout;
	layout["tomthread"] = tomthread_layout;
	layout["tablet"] = tablet_layout;
	layout["contestdesc"] = contest_layout;

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
		{"How does this Centerracom Solutions product compare to its direct competitors?",
		"The Centerracom Solutions product is considerably superior", "The Centerracom Solutions product is somewhat superior",
		"The Centerracom Solutions product is slightly superior", "The Centerracom Solutions product is slightly inferior",
		"The Centerracom Solutions product is somewhat inferior", "The Centerracom Solutions product is completely inferior"},
		{"I can see myself using this Centerracom Solutions product at my organization or workplace...",
		"Very frequently", "Somewhat frequently", "Occasionally", "Somewhat infrequently", "Very infrequently"},
		{"How well did the product meet your expectations in terms of design and functionality?",
		"The product completely met my expectations", "The product mostly met my expectations", "The product partially met my expectations",
		"The product did not meet all of my expectations", "The product completely failed to meet my expectations"},
		{"Was the user interface of the software clear and easy to use?",
		"Very easy to use", "Somewhat easy to use", "Neither easy nor difficult to use", "Somewhat difficult to use", "Very difficult to use"},
		{"After participating in this software trial, are you interested in purchasing this product?",
		"Very interested", "Somewhat interested", "Neutral", "Not very interested", "Not at all interested"},
		{"With this product, has Centerracom Solutions continued to deliver on its promise of excellence in software engineering?",
		"Centerracom Solutions has absolutely continued to deliver on its promise",
		"Centerracom Solutions has mostly continued to deliver on its promise", "Centerracom Solutions has continued to deliver on its promise, despite some setbacks",
		"Centerracom Solutions has had some difficulties continuing to deliver on its promise",
		"Centerracom Solutions has failed in some ways to deliver on its promise",
		"Centerracom Solutions has completely failed to deliver on its promise"}
	};

	survey_questions2 =
	{
		{"On a scale of 1-10, rate how well each of the following words apply to our software:", "innovative"},
		{"", "lackadaisical"},
		{"", "performant"},
		{"", "elegant"},
		{"", "refined"},
		{"", "polished"},
		{"", "jocular"},
		{"", "vivacious"},
		{"", "skillful"},
		{"", "accessible"},
		{"", "stable"},
		{"", "reliable"},
		{"", "compassionate"},
		{"", "prescient"},
		{"", "stoic"},
		{"", "fulfilling"},
		{"", "luxurious"},
		{"", "inquisitive"},
		{"", "expansive"},
		{"", "active"}
	};

	survey_questions3 =
	{
		{"Our software is critical to your competitive advantage in emerging markets",
		"Strongly agree", "Agree", "Somewhat agree", "Somewhat disagree", "Disagree", "Strongly disagree"},
		{ "Centerracom Solutions has a robust, extensible growth strategy that will continue to allow us to provide mission-critical solutions to our partners and customers.",
		"Strongly agree", "Agree", "Somewhat agree", "Somewhat disagree", "Disagree", "Strongly disagree" },
		{ "This Centerracom Solutions product will integrate well with your existing quality assurance procedures.",
		"Strongly agree", "Agree", "Somewhat agree", "Somewhat disagree", "Disagree", "Strongly disagree" },
		{ "I feel that Centerracom Solutions is responsive to my concerns as a customer and takes my needs into account when making decisions regarding the future of its products and services.",
		"Strongly agree", "Agree", "Somewhat agree", "Somewhat disagree", "Disagree", "Strongly disagree" },
		{ "I am interested in Centerracom's enterprise volume licensing deals, since multiple parts of my organization could benefit from Centerracom products.",
		"Strongly agree", "Agree", "Somewhat agree", "Somewhat disagree", "Disagree", "Strongly disagree" },
		{ "I would consider going to work for Centerracom.",
		"Strongly agree", "Agree", "Somewhat agree", "Somewhat disagree", "Disagree", "Strongly disagree" },
		{ "The Centerracom trial product I used is now my favorite software product",
		"Strongly agree", "Agree", "Somewhat agree", "Somewhat disagree", "Disagree", "Strongly disagree" },
		{ "I am dedicated to making the best decisions possible for the IT department within my larger organization.",
		"Strongly agree", "Agree", "Somewhat agree", "Somewhat disagree", "Disagree", "Strongly disagree" },
		{ "I enjoyed my time with the trial software product.",
		"Strongly agree", "Agree", "Somewhat agree", "Somewhat disagree", "Disagree", "Strongly disagree" },
		{ "I am filled with evil desires.",
		"Strongly agree", "Agree", "Somewhat agree", "Somewhat disagree", "Disagree", "Strongly disagree" },
		{ "Centerracom Solutions continues to be a leader in robust, sustainable enterprise software.",
		"Strongly agree", "Agree", "Somewhat agree", "Somewhat disagree", "Disagree", "Strongly disagree" },
		{ "My corporation will benefit from a closer relationship with Centerracom Solutions moving forward.",
		"Strongly agree", "Agree", "Somewhat agree", "Somewhat disagree", "Disagree", "Strongly disagree" },
		{ "I would recommend Centerracom products to friends and family for personal use.",
		"Strongly agree", "Agree", "Somewhat agree", "Somewhat disagree", "Disagree", "Strongly disagree" },
		{ "My annual income is more than $60,000.",
		"Strongly agree", "Agree", "Somewhat agree", "Somewhat disagree", "Disagree", "Strongly disagree" },
		{ "It makes sense that Centerracom Solutions was rated one of the year's best employers in a national survey.",
		"Strongly agree", "Agree", "Somewhat agree", "Somewhat disagree", "Disagree", "Strongly disagree" },
		{ "I find it difficult to forgive people.",
		"Strongly agree", "Agree", "Somewhat agree", "Somewhat disagree", "Disagree", "Strongly disagree" },
	};

	prev_mouse_x = 0;
	prev_mouse_y = 0;
    history_index = -1;
	prev_website = "";
}

string ScrollPanel::replace_whitespace(string str)
{
	string ret = "";
	bool whitespace = false;
	for (int i = 0; i < str.length(); ++i)
	{
		if (isspace(str[i]))
			whitespace = true;

		else
		{
			if (whitespace)
				ret += '+';

			ret += str[i];
			whitespace = false;
		}
	}

	return ret;
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
			bool underline = (components.size() >= 6 && components[5] == "u") ? true : false;
			links.push_back(Link(Rect(atof(components[0].c_str()) + cur_x, (cur_y - image_height) + atof(components[1].c_str()), atof(components[2].c_str()) + cur_x, (cur_y - image_height) + atof(components[3].c_str())), components[4], underline));
		}

		cur_y -= TextureManager::get_height(cur_layout[i].second) + 20;
		cur_x = start_x;
	}

	if (cur_y < 0)
		resize_page(-(cur_y - 35));
}

void ScrollPanel::resize_page(float delta)
{
	sum_of_heights += delta;
	if (sum_of_heights <= 1080.0 && textures[0] != "eula.png")
	{
		scroll_bar->is_visible = false;
		scroll_bar->y2 = scroll_bar->max_y;
		sum_of_heights = 1080.0;
	}

	else
	{
		if (!scroll_bar->is_visible)
		{
			scroll_bar->is_visible = true;
			scroll_bar->y2 = scroll_bar->max_y;
		}

		else
			scroll_bar->is_visible = true;

		int height = ((y2 - y1) / ((float)sum_of_heights)) * scroll_bar->range();
		if (delta > 0)
			scroll_bar->y1 = scroll_bar->y2 - height;

		else
		{
			scroll_bar->y1 -= height - (scroll_bar->y2 - scroll_bar->y1);
			if (scroll_bar->y1 < scroll_bar->min_y)
			{
				scroll_bar->y2 += scroll_bar->min_y - scroll_bar->y1;
				scroll_bar->y1 = scroll_bar->min_y;
			}
		}
	}
}

bool ScrollPanel::upload_image_file(string file_name, string folder_name)
{
	File file = parent->read_image_file(file_name, folder_name);
	if (file.xsize == 0)
		return false;

	parent->uploaded_image = file; //TODO this should probably be free'd at some point?
	return true;
}

void ScrollPanel::restart_game()
{
	change_website("search");
	parent->process_control_changes();
	change_website("game");
	parent->process_control_changes();
}

void ScrollPanel::add_children(string website_name, bool skip_loading)
{
	parent->read_email[website_name + "bold.png"] = true;
	//TODO sometimes spinner will randomly flicker even when it's not supposed to
	if (website_name == "search")
	{
		if (parent->read_email["rosecolored4bold.png"] && !parent->reached_help_center)
		{
			auto text_field = make_unique<TextField>(978, 1080 - 564, 978 + 393, (1080 - 564) + 42, "searchbar.png", parent, application);
			text_field->font_size = 64;
			text_field->maximum_length = 500;
			text_field->text = "help";
			auto text_field_ptr = text_field.get();
			children.push_back(text_field.get());
			auto button = make_unique<Button>(978 + 393, 1080 - 564, 978 + 393 + 110, (1080 - 564) + 42, "searchbutton.png", BROWSER,
				[this, text_field_ptr]()
			{
				if (text_field_ptr->text == "help")
				{
					white_cover_alpha.step();
					white_cover_apex = [this, text_field_ptr]()
					{
						retain_white_cover_alpha = true;
						change_website("searchresults", true, true);
						retain_white_cover_alpha = false;
					};
				}

				else if (text_field_ptr->text != "")
				{
					white_cover_alpha.step();
					white_cover_apex = [this, text_field_ptr]()
					{
						retain_white_cover_alpha = true;
						change_website("search/search?q=" + replace_whitespace(text_field_ptr->text), true, true);
						retain_white_cover_alpha = false;
					};
				}
			}, []() {}, "", "genlight.png", "gendark.png");
			auto button_ptr = button.get();
			text_field->on_enter_function = [button_ptr, this]() { this->parent->set_cursor("cursor.png"); button_ptr->release_function(); };
			text_field_ptr->drawn_by_browser = button_ptr->drawn_by_browser = true;
			parent->to_be_added.push_back(std::move(text_field));
			button->start_flashing();
			children.push_back(button.get());
			parent->to_be_added.push_back(std::move(button));
		}
		
		else
		{
			auto text_field = make_unique<TextField>(978, 1080 - 564, 978 + 393, (1080 - 564) + 42, "searchbar.png", parent, application);
			text_field->font_size = 64;
			text_field->maximum_length = 500;
			children.push_back(text_field.get());
			auto search_ptr = text_field.get();
			auto button = make_unique<Button>(978 + 393, 1080 - 564, 978 + 393 + 110, (1080 - 564) + 42, "searchbutton.png", BROWSER,
				[this, search_ptr]()
			{
				if (search_ptr->text != "")
				{
					white_cover_alpha.step();
					white_cover_apex = [this, search_ptr]()
					{
						if (parent->reached_space_game && !parent->beat_space_game)
						{
							retain_white_cover_alpha = false;
							change_website("game", true, true);
						}

						else
						{
							retain_white_cover_alpha = true;
							change_website("search/search?q=" + replace_whitespace(search_ptr->text), true, true);
							retain_white_cover_alpha = false;
						}
					};
				}
			}, []() {}, "", "genlight.png", "gendark.png");
			auto button_ptr = button.get();
			if (parent->reached_space_game && !parent->beat_space_game)
			{
				text_field->text = "Galactic Conquest";
				button->start_flashing();
			}

			text_field->on_enter_function = [button_ptr, this]() { this->parent->set_cursor("cursor.png"); button_ptr->release_function(); };
			search_ptr->drawn_by_browser = button_ptr->drawn_by_browser = true;
			parent->to_be_added.push_back(std::move(text_field));
			children.push_back(button.get());
			parent->to_be_added.push_back(std::move(button));
		}
	}

	if (website_name == "search/search?q=")
	{
		auto cover = make_unique<ScreenElement>(0, 0, 1920, 1080, "searchspinner.png", BROWSER);
		auto cover_ptr = cover.get();
		children.push_back(cover_ptr);
		auto spinner = make_unique<Spinner>(10, 0, glutGet(GLUT_WINDOW_WIDTH) / 2.0, glutGet(GLUT_WINDOW_HEIGHT) / 2.0, BROWSER);
		children.push_back(spinner.get());
		if (skip_loading)
		{
			spinner->frames = 200;
			spinner->is_visible = false;
			cover->is_visible = false;
		}

		spinner->drawn_by_browser = cover->drawn_by_browser = true;
		parent->to_be_added.push_back(std::move(spinner));
		parent->to_be_added.push_back(std::move(cover));
		animation_function = [this, cover_ptr, skip_loading]()
		{
			Spinner* spinner = NULL;
			if (this->children.size() > 1)
				spinner = dynamic_cast<Spinner*>(this->children[1]);

			if (spinner != NULL)
			{
				if (spinner->frames > 150 && white_cover_alpha.current_value == 0.0)
				{
					white_cover_alpha.step();
					white_cover_apex = [this, spinner, cover_ptr, skip_loading]()
					{
						spinner->marked_for_deletion = true;
						cover_ptr->marked_for_deletion = true;
						children.clear();
						this->parent->process_control_changes();
					};

					if (skip_loading)
					{
						white_cover_apex();
						white_cover_alpha.current_value = 0;
					}
				}
			}
		};
	}

	if (website_name == "error")
	{
		auto error_message = make_unique<ScreenElement>(607, 1080 - 824, -1, -1, "404message.png", BROWSER);
		error_message->alpha = 0;
		auto err_ptr = error_message.get();
		children.push_back(err_ptr);
		parent->to_be_added.push_back(std::move(error_message));
		auto error_face = make_unique<ReflectedObject>(850, 1080 - 466, -1, -1, "errorface.png", BROWSER, 0, this);
		children.push_back(error_face.get());
		parent->to_be_added.push_back(std::move(error_face));
		animation_function = [this, err_ptr]()
		{
			if (err_ptr->alpha < 1.0)
				err_ptr->alpha += 0.01;

			if (myframes % 50 == 0)
			{
				int nx1 = rand_in_int(100, 1500);
				int ny1 = rand_in_int(700, 900);
				auto elem = make_unique<ReflectedObject>(nx1, ny1, nx1 + 119, ny1 + 70, "404.png", BROWSER, 1, this);
				elem->browser_child = true;
				children.push_back(elem.get());
				parent->to_be_added.push_back(std::move(elem));
			}
		};
	}

	if (website_name == "triallink")
	{
		parent->whitelist.push_back("download_trial");
	}

	if (website_name == "thread52")
	{
		if (!parent->playing_escape_sound)
		{
			parent->contest_finished = true;
			parent->required_target = "nenda";
		}
	}

	if (website_name == "thread11")
	{
		if (!parent->visited_site["thread11"])
			parent->start_playing_escape_sound("thesounds.wav");
	}

	if (website_name == "useful")
	{
		auto message = make_unique<ScreenElement>(460, 1080 - 836, -1, -1, "usefulmessage" + to_string(parent->day_number) + ".png", BROWSER);
		children.push_back(message.get());
		parent->to_be_added.push_back(std::move(message));
	}

	if (website_name == "ain" || website_name == "advertising" || website_name == "purchase" || website_name == "purchasecomplete")
	{
		if (parent->entered_contest)
		{
			auto banner = make_unique<ScreenElement>(38, 746, 509, 809, "bannerad.png", BROWSER);
			banner->never_focus = true;
			children.push_back(banner.get());
			parent->to_be_added.push_back(std::move(banner));
			for (int i = 0; i < links.size(); ++i)
			{
				if (links[i].target == "advertising")
					links[i].target = "centerracom";
			}

			if (parent->day_number == 3 && find(parent->whitelist.begin(), parent->whitelist.end(), "centerracom") == parent->whitelist.end())
			{
				parent->whitelist.push_back("centerracom");
				parent->whitelist.push_back("centerracompartners");
				parent->whitelist.push_back("centerracomtrial");
				parent->whitelist.push_back("centerracomcontact");
				parent->whitelist.push_back("funcorner");
			}
		}
	}

	if (website_name == "ain" || website_name == "e1" || website_name == "psychologist")
	{
		if (website_name == "psychologist")
		{
			bool do_save = false;
			if (!parent->reached_psychologist)
				do_save = true;

			parent->reached_psychologist = true;
			if (do_save)
				save_game(parent->save_slot);
		}

		vector<string> empty_labels = { "", "", "", "", "" };
		create_radio_button_group(1605, 1080 - 694, 5, false, empty_labels, 28);
		if (!parent->submitted_poll_of_the_day)
		{
			create_survey_submit_button(1649, 1080 - 827, [this]()
			{
				for (int i = 0; i < this->children.size(); ++i)
				{
					if (this->children[i]->name == "submitvote.png")
						this->children[i]->is_visible = false;
				}

				auto new_message = make_unique<ScreenElement>(1607, 1080 - 827 + this->offset, 1607 + 152, 1080 - 827 + 9 + this->offset, "votehasbeensubmitted.png", BROWSER);
				new_message->never_focus = true;
				this->children.push_back(new_message.get());
				this->parent->to_be_added.push_back(std::move(new_message));
				this->parent->submitted_poll_of_the_day = true;
			}, []() {}, "submitvote.png", "invisible.png", "invisible.png");
		}

		else
		{
			auto new_message = make_unique<ScreenElement>(1607, 1080 - 827 + this->offset, 1607 + 152, 1080 - 827 + 9 + this->offset, "votehasbeensubmitted.png", BROWSER);
			new_message->never_focus = true;
			this->children.push_back(new_message.get());
			this->parent->to_be_added.push_back(std::move(new_message));
		}

		auto username_field = make_unique<TextField>(1683, 1080 - 434, 1837, 1080 - 434 + 30, "textfield.png", parent, BROWSER);
		auto password_field = make_unique<TextField>(1683, 1080 - 476, 1837, 1080 - 476 + 30, "textfield.png", parent, BROWSER);
		username_field->tab_target = password_field.get();
		password_field->tab_target = username_field.get();
		password_field->password = true;
		children.push_back(username_field.get());
		children.push_back(password_field.get());
		parent->to_be_added.push_back(std::move(username_field));
		parent->to_be_added.push_back(std::move(password_field));
		auto error = make_unique<ExpandableImage>(1614, 1080 - 532 + this->offset, 1614 + 182, 1080 - 532 + this->offset, "pleaseenter.png", BROWSER, 12);
		auto err_ptr = error.get();
		auto login_button = make_unique<Button>(1651, 1080 - 520 + this->offset, -1, -1, "ainlogin.png", BROWSER, [err_ptr]()
		{
			if (err_ptr->y2 - err_ptr->y1 == 0)
				err_ptr->toggle();
		}, []() {});

		children.push_back(err_ptr);
		children.push_back(login_button.get());
		parent->to_be_added.push_back(std::move(error));
		parent->to_be_added.push_back(std::move(login_button));
	}

	if (website_name == "purchase")
	{
		auto field1 = make_unique<TextField>(479, 1080 - 537, 679, 1080 - 537 + 30, "textfield.png", parent, BROWSER);
		auto field2 = make_unique<TextField>(479, 1080 - 537 - 50, 679, 1080 - 537 + 30 - 50, "textfield.png", parent, BROWSER);
		//auto field3 = make_unique<TextField>(479, 1080 - 537 - 100, 679, 1080 - 537 + 30 - 100, "textfield.png", parent, BROWSER);
		auto field3 = make_unique<TextField>(479, 1080 - 537 - 100, 515 + 24, 1080 - 537 + 30 - 100, "textfield.png", parent, BROWSER);
		auto field6 = make_unique<TextField>(543, 1080 - 537 - 100, 579 + 24, 1080 - 537 + 30 - 100, "textfield.png", parent, BROWSER);
		auto field7 = make_unique<TextField>(607, 1080 - 537 - 100, 679, 1080 - 537 + 30 - 100, "textfield.png", parent, BROWSER);
		auto field4 = make_unique<TextField>(479, 1080 - 537 - 150, 679, 1080 - 537 + 30 - 150, "textfield.png", parent, BROWSER);
		field3->filler_text = "MM";
		field6->filler_text = "DD";
		field7->filler_text = "YYYY";
		auto field5 = make_unique<TextField>(479, 1080 - 537 - 200, 679, 1080 - 537 + 30 - 200, "textfield.png", parent, BROWSER);
		auto ptr1 = field1.get();
		auto ptr2 = field2.get();
		auto ptr3 = field3.get();
		auto ptr4 = field4.get();
		auto ptr5 = field5.get();
		auto ptr6 = field6.get();
		auto ptr7 = field7.get();
		field1->tab_target = ptr2;
		field2->tab_target = ptr3;
		field3->tab_target = ptr6;
		field4->tab_target = ptr5;
		field6->tab_target = ptr7;
		field7->tab_target = ptr4;
		field5->tab_target = ptr1;
		set<unsigned char> allowed{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
		field3->allowed_characters = allowed;
		field6->allowed_characters = allowed;
		field7->allowed_characters = allowed;
		field5->allowed_characters = allowed;
		field3->maximum_length = 2;
		field6->maximum_length = 2;
		field7->maximum_length = 4;
		field5->maximum_length = 16;
		auto error = make_unique<ExpandableImage>(500, 1080 - 860 + this->offset, 500 + 159, 1080 - 860 + this->offset, "allfields.png", BROWSER, 11);
		auto err_ptr = error.get();
		auto email_error = make_unique<ExpandableImage>(685, 1080 - 537 + 30 - 150 - 10 + this->offset, 890, 1080 - 537 + 30 - 150 - 10 + this->offset, "emailaddress.png", BROWSER, 10);
		auto email_error_ptr = email_error.get();
		auto date_error = make_unique<ExpandableImage>(685, 1080 - 537 + 30 - 100 - 10 + this->offset, 833, 1080 - 537 + 30 - 100 - 10 + this->offset, "validdate.png", BROWSER, 10);
		auto date_error_ptr = date_error.get();
		auto card_error = make_unique<ExpandableImage>(685, 1080 - 537 + 30 - 200 - 10 + this->offset, 922, 1080 - 537 + 30 - 200 - 10 + this->offset, "validcard.png", BROWSER, 10);
		auto card_error_ptr = card_error.get();
		auto purchase_button = make_unique<Button>(328, 1080 - 878, -1, -1, "clickheretopurchase.png", BROWSER, [ptr1, ptr2, ptr3, ptr4, ptr5, ptr6, ptr7, err_ptr, email_error_ptr, date_error_ptr, card_error_ptr, this]()
		{
			int month = atoi(ptr3->text.c_str());
			int day = atoi(ptr6->text.c_str());
			int year = atoi(ptr7->text.c_str());
			bool found_error = false;
			err_ptr->reset();
			email_error_ptr->reset();
			date_error_ptr->reset();
			card_error_ptr->reset();
			if (ptr1->text == "" || ptr2->text == "" || ptr3->text == "" || ptr4->text == "" || ptr5->text == "" || ptr6->text == "" || ptr7->text == "")
			{
				err_ptr->toggle();
				found_error = true;
			}

			if (!ptr4->validate_email_address())
			{
				email_error_ptr->toggle();
				found_error = true;
			}

			if (month <= 0 || month > 12 || day <= 0 || day > 31)
			{
				date_error_ptr->toggle();
				found_error = true;
			}

			if (!ptr5->validate_credit_card())
			{
				card_error_ptr->toggle();
				found_error = true;
			}

			if (!found_error)
			{
				this->start_loading_site("purchasecomplete", true);
				will_force_through = true;
			}
		}, []() {});
		children.push_back(field1.get());
		children.push_back(field2.get());
		children.push_back(field3.get());
		children.push_back(field4.get());
		children.push_back(field5.get());
		children.push_back(field6.get());
		children.push_back(field7.get());
		children.push_back(err_ptr);
		children.push_back(email_error_ptr);
		children.push_back(date_error_ptr);
		children.push_back(card_error_ptr);
		children.push_back(purchase_button.get());
		parent->to_be_added.push_back(std::move(field1));
		parent->to_be_added.push_back(std::move(field2));
		parent->to_be_added.push_back(std::move(field3));
		parent->to_be_added.push_back(std::move(field4));
		parent->to_be_added.push_back(std::move(field5));
		parent->to_be_added.push_back(std::move(field6));
		parent->to_be_added.push_back(std::move(field7));
		parent->to_be_added.push_back(std::move(error));
		parent->to_be_added.push_back(std::move(email_error));
		parent->to_be_added.push_back(std::move(card_error));
		parent->to_be_added.push_back(std::move(date_error));
		parent->to_be_added.push_back(std::move(purchase_button));
	}

	if (website_name == "helphome")
	{
		auto connect_button = make_unique<Button>(687, 1080 - 1316, -1, -1, "connect.png", BROWSER, []() {}, []() {}, "", "invisible.png", "connect2.png");
		auto connect_ptr = connect_button.get();
		connect_button->release_function = [this, connect_ptr]()
		{
			connect_ptr->marked_for_deletion = true;
			children.clear();
			auto loading_bar = make_unique<LoadingBar>(557, 1080 - 1316 + this->offset, -1, -1, "connectbar.png", BROWSER);
			loading_bar->browser_child = true;
			this->children.push_back(loading_bar.get());
			this->parent->to_be_added.push_back(std::move(loading_bar));
			this->animation_function = [this]()
			{
				LoadingBar* loading_bar = dynamic_cast<LoadingBar*>(this->children[0]);
				{
					if (loading_bar != NULL)
					{
						if (loading_bar->frames >= 60 * 4)
						{
							loading_bar->marked_for_deletion = true;
							children.clear();
							auto help_center = make_unique<HelpCenter>(537, 1080 - 1443 + this->offset, 976, 1080 - 1098 + this->offset, "red.png", parent, application);
							help_center->browser_child = true;
							help_center->text_field_ptr->x1 = 537;
							help_center->text_field_ptr->y1 = 1080 - 1503 + this->offset;
							help_center->text_field_ptr->x2 = 836;
							help_center->text_field_ptr->y2 = 1080 - 1463 + this->offset;
							help_center->text_field_ptr->browser_child = true;
							help_center->text_field_ptr->always_highlight = true;
							help_center->button_ptr->x1 = 836;
							help_center->button_ptr->y1 = 1080 - 1503 + this->offset;
							help_center->button_ptr->x2 = help_center->button_ptr->y2 = -1;
							help_center->button_ptr->need_resize = true;
							help_center->button_ptr->browser_child = true;
							help_center->always_highlight = true;
							children.push_back(help_center.get());
							children.push_back(help_center->button_ptr);
							children.push_back(help_center->text_field_ptr);
							parent->to_be_added.push_back(std::move(help_center));
							auto now_connected = make_unique<ScreenElement>(1224, 1080 - 1076 + this->offset, -1, -1, "nowconnected.png", application);
							children.push_back(now_connected.get());
							parent->to_be_added.push_back(std::move(now_connected));
							parent->reached_help_center = true;
							parent->process_control_changes();
						}
					}
				}
			};
		};
		children.push_back(connect_button.get());
		parent->to_be_added.push_back(std::move(connect_button));
		parent->whitelist.push_back("helphome");
	}

	if (website_name == "helpcenter")
	{
		auto help_center = make_unique<HelpCenter>(500, 200, 850 + 162, 950, "helpbackground.png", parent, application); //(0.0, 56.0, 1900.0, 1000.0, "white.png", parent, application);
		children.push_back(help_center.get());
		children.push_back(help_center->button_ptr);
		children.push_back(help_center->text_field_ptr);
		parent->to_be_added.push_back(std::move(help_center)); //browser_child controls get added in reverse order right now
		auto help_message = make_unique<ScreenElement>(1100, 920, 1100 + 162, 950, "helpmessage.png", application);
		children.push_back(help_message.get());
		parent->to_be_added.push_back(std::move(help_message));
	}

	if (website_name == "survey")
	{
		auto header = make_unique<ScreenElement>(153, 1080 - 232, -1, -1, "carheader.png", BROWSER);
		children.push_back(header.get());
		parent->to_be_added.push_back(std::move(header));
		int start_y = 750;
		for (int i = 0; i < survey_questions1.size(); ++i)
			start_y = create_radio_button_group(200, start_y - 50, survey_questions1[i].size(), false, survey_questions1[i]) - 100;

		resize_page(-(start_y - 100));
		create_survey_submit_button(1920 - 400, 1080 - sum_of_heights + 100, [this]() 
		{
			this->start_loading_site("survey2");
			this->will_force_through = true;
		}, [this]() {this->make_popup("Error: Left questions blank.");});
	}

	if (website_name == "survey2")
	{
		auto header = make_unique<ScreenElement>(153, 1080 - 232, -1, -1, "carheader.png", BROWSER);
		children.push_back(header.get());
		parent->to_be_added.push_back(std::move(header));
		auto title_label = make_unique<InfoText>(200 - 10, 750, 200 - 10, 750, "invisible.png", "On a scale of 1 to 10 (left to right), how well do the following words describe the trial product you tested?", BROWSER);
		children.push_back(title_label.get());
		int start_y = 750;
		for (int i = 0; i < survey_questions2.size(); ++i)
			start_y = create_radio_button_group(200, start_y - 50, 11, true, survey_questions2[i]);

		resize_page(-(start_y - 100));
		create_survey_submit_button(1920 - 400, 1080 - sum_of_heights + 100, [this]()
		{
			this->start_loading_site("survey3");
			this->will_force_through = true;
		}, [this]() {this->make_popup("Error: Left questions blank.");});
		parent->to_be_added.push_back(std::move(title_label));
	}

	if (website_name == "survey3")
	{
		auto header = make_unique<ScreenElement>(153, 1080 - 232, -1, -1, "carheader.png", BROWSER);
		children.push_back(header.get());
		parent->to_be_added.push_back(std::move(header));
		auto title_label = make_unique<InfoText>(200 - 10, 850, 200 - 10, 850, "invisible.png", "Indicate how much you agree with the following statements:", BROWSER);
		children.push_back(title_label.get());
		int start_y = 750;
		for (int i = 0; i < survey_questions3.size(); ++i)
			start_y = create_radio_button_group(200, start_y - 50, survey_questions3[i].size(), false, survey_questions3[i]) - 100;

		resize_page(-(start_y - 100));
		create_survey_submit_button(1920 - 400, 1080 - sum_of_heights + 100, [this]()
		{
			this->start_loading_site("survey4");
			this->will_force_through = true;
		}, [this]() {this->make_popup("Error: Left questions blank.");});
		parent->to_be_added.push_back(std::move(title_label));
	}

	if (website_name == "survey4")
	{
		auto header = make_unique<ScreenElement>(153, 1080 - 232, -1, -1, "carheader.png", BROWSER);
		children.push_back(header.get());
		parent->to_be_added.push_back(std::move(header));
		auto title_label = make_unique<InfoText>(200 - 10, 750, 200 - 10, 750, "invisible.png", "Thank you for completing the survey.", BROWSER);
		children.push_back(title_label.get());
		parent->to_be_added.push_back(std::move(title_label));
		parent->delist("centerracom");
		parent->delist("centerracomtrial");
		parent->delist("centerracomcontact");
		parent->delist("centerracompartners");
		parent->delist("funcorner");
		parent->delist("survey");
		parent->delist("survey2");
		parent->delist("survey3");
		parent->delist("survey4");
		parent->delist("ain");
		parent->delist("e1");
		parent->delist("advertising");
		parent->delist("purchase");
		parent->delist("purchasecomplete");
		parent->delist("psychologist");
		parent->websites["reddit"] = { "sleepy2.png" };
		parent->whitelist.push_back("hcthread");
		parent->whitelist.push_back("reddit");
		parent->day_number++;
		parent->submitted_poll_of_the_day = false;
		//parent->websites["useful"] = { "useful40.png" };
		parent->sequences["sequencereddit"] = { "hcthread" };
		parent->whitelist.push_back("imagehost");
		parent->whitelist.push_back("forumhome");
		parent->whitelist.push_back("forumthreads");
		save_game(parent->save_slot);
	}

	if (website_name == "contestupload")
	{
		auto selection_field = make_unique<TextField>(110, 1080 - 215, 328, 1080 - 215 + 30, "textfield.png", parent, BROWSER);
		selection_field_ptr = selection_field.get();
		children.push_back(selection_field_ptr);
		parent->to_be_added.push_back(std::move(selection_field));
	}

	if (website_name == "centerracomtrial")
	{
		auto text_field = make_unique<TextField>(650, -130 + 415, 650 + 229, -130 + 415 + 40, "textfieldblocky.png", parent, application, [](){});
		auto text_field_ptr = text_field.get();
		auto button = make_unique<Button>(900, -130 + 415, 1000, -100 + 415, "okblocky.png", application,
			[this, text_field_ptr]()
		{
			if (text_field_ptr->text == "xj5555585@nenda.en.os.esn")
			{
				bool do_save = false;
				if (!parent->entered_right_email_address)
					do_save = true;

				this->parent->entered_right_email_address = true;
				if (do_save)
					save_game(parent->save_slot);
			}

			if (text_field_ptr->text != "")
			{
				if (text_field_ptr->validate_email_address())
				{
					this->start_loading_site("centerracomthankyou");
					this->will_force_through = true;
				}

				else
				{
					this->start_loading_site("centerracominvalid");
					this->will_force_through = true;
				}
			}
		}, [](){}, "", "invisible.png", "okblockypressed.png");

		children.push_back(text_field_ptr);
		children.push_back(button.get());
		parent->to_be_added.push_back(std::move(text_field));
		parent->to_be_added.push_back(std::move(button));
	}

	if (website_name == "thankyouforuploading" && !parent->entered_contest)
	{
		parent->entered_contest = true;
		parent->day_number++;
		parent->submitted_poll_of_the_day = false;
		parent->websites["ain"] = { "ain3.png" };
		parent->sequences["sequenceain"].pop_back();
		parent->delist("walter1");
		parent->delist("contestupload");
		parent->delist("thankyouforuploading");
		parent->delist("psychologist");
		save_game(parent->save_slot);
	}
	
	if (website_name == "reddit" || website_name == "tablet" || website_name == "conversation" || website_name == "tomthread" || website_name == "contestdesc" || website_name == "contestresults" || website_name == "thankyouforuploading"
		|| website_name == "hcthread" || website_name == "decryptthread")
	{
		auto name_field = make_unique<TextField>(1365, 1080 - 112, 1503, 1080 - 82, "white.png", parent, BROWSER);
		auto password_field = make_unique<TextField>(1616, 1080 - 112, 1754, 1080 - 82, "white.png", parent, BROWSER);
		password_field->password = true;
		auto nameptr = name_field.get();
		auto passptr = password_field.get();
		nameptr->tab_target = passptr;
		passptr->tab_target = nameptr;
		children.push_back(name_field.get());
		children.push_back(password_field.get());
		parent->to_be_added.push_back(std::move(name_field));
		parent->to_be_added.push_back(std::move(password_field));
	}

	if (website_name == "hcthread")
	{
		parent->whitelist.push_back("forumhome");
		parent->whitelist.push_back("forumthreads");
		parent->whitelist.push_back("imagehost");
	}

	if (website_name == "thread11")
	{
		parent->whitelist.push_back("thread11");
		parent->whitelist.push_back("thread12");
	}

	if (website_name == "thread21")
	{
		parent->whitelist.push_back("thread21");
		parent->whitelist.push_back("thread22");
	}

	if (website_name == "thread31")
	{
		parent->whitelist.push_back("thread31");
		parent->whitelist.push_back("thread32");
	}

	if (website_name == "thread41")
		parent->whitelist.push_back("thread41");

	if (website_name == "thread51")
		parent->whitelist.push_back("thread51");

	if (website_name == "walter3")
		parent->whitelist.push_back("arcade");

	if (website_name == "walter2")
	{
		results_counter = 0;
		auto image = make_unique<StaticCanvas>(969, 1080 - 2103, 969 + 535, (1080 - 2103) + 616, "", BROWSER, parent);
		image->never_focus = true;
		children.push_back(image.get());
		parent->to_be_added.push_back(std::move(image));
		int cur_x = 721;
		int cur_y = 1080 - 831;
		for (int i = 0; i < 10; ++i)
		{
			auto cover = make_unique<ScreenElement>(cur_x, cur_y, cur_x + 230, cur_y + 562, "black.png", BROWSER);
			cover->never_focus = true;
			auto cover_ptr = cover.get();
			children.push_back(cover_ptr);
			parent->to_be_added.push_back(std::move(cover));
			auto button = make_unique<Button>(cur_x + 15, cur_y + 571, cur_x + 15 + 100, cur_y + 571 + 24, "showcomments1.png", BROWSER,
				[this, i, cover_ptr]()
			{
				if (cover_ptr->is_visible == false || cover_ptr->name != "black.png")
				{
					cover_ptr->is_visible = true;
					cover_ptr->name = "black.png";
				}

				else
				{
					if (i == 5 && results_counter == 0)
					{
						results_counter++;
						cover_ptr->is_visible = false;
					}

					else if (results_counter == 2 && i != 5 && !parent->showed_walter)
					{
						results_counter++;
						cover_ptr->name = "walterface.png";
						parent->showed_walter = true;
						parent->required_target = "walter3";
						parent->whitelist.push_back("walter3");
					}

					else
					{
						cover_ptr->is_visible = false;
						if (results_counter != 0 && i != 5)
							results_counter++;
					}
				}
			}, []() {}, "", "showcomments2.png", "showcomments3.png");
			children.push_back(button.get());
			parent->to_be_added.push_back(std::move(button));
			if (i % 2 == 0)
				cur_x += 784;

			else
			{
				cur_y -= 636;
				cur_x -= 784;
			}
		}
	}

	if (website_name == "contestresults")
	{
		/*auto static_canvas = make_unique<StaticCanvas>(100, 800 - parent->uploaded_image.ysize, 100 + parent->uploaded_image.xsize, 800, "", BROWSER, parent->uploaded_image.image);
		static_canvas->never_focus = true;
		children.push_back(static_canvas.get());
		parent->to_be_added.push_back(std::move(static_canvas));*/

		auto header = make_unique<ScreenElement>(0, 1080 - 221, 1920, 1080, "contestheader.png", BROWSER);
		header->never_focus = true;
		children.push_back(header.get());
		parent->to_be_added.push_back(std::move(header));
		vector<string> images = { "contest1.png", "contest3.png", "contest2.png", "", "contest4.png" };
		vector<string> comments = { "comments0.png", "comments1.png", "comments2.png", "comments3.png", "comments4.png" };
		int cur_y = 700;
		for (int i = 0; i < images.size(); ++i)
		{
			int inc_height, actual_width, actual_height;
			if (i == 3)
			{
				actual_width = min(500, parent->uploaded_image.xsize);
				actual_height = min(500, parent->uploaded_image.ysize);
				auto image = make_unique<StaticCanvas>(300, cur_y, 300 + actual_width, cur_y + actual_height - 20, "", BROWSER, parent);
				image->never_focus = true;
				children.push_back(image.get());
				parent->to_be_added.push_back(std::move(image));
				auto second_place = make_unique<ScreenElement>(300, cur_y + actual_height - 12, 300 + 96, cur_y + actual_height, "secondplace.png", BROWSER);
				second_place->never_focus = true;
				children.push_back(second_place.get());
				parent->to_be_added.push_back(std::move(second_place));
			}
			
			else
			{
				auto image = make_unique<ScreenElement>(300, cur_y, 300 + parent->texture_manager->get_width(images[i]), cur_y + parent->texture_manager->get_height(images[i]), images[i], BROWSER);
				image->never_focus = true;
				children.push_back(image.get());
				parent->to_be_added.push_back(std::move(image));
			}

			if (i == 2)
				inc_height = min(500, parent->uploaded_image.ysize);

			else if (i < images.size() - 1)
				inc_height = parent->texture_manager->get_height(images[i + 1]);

			else inc_height = 0;

			auto expandable = make_unique<ExpandableImage>(300, cur_y - 24, 300 + parent->texture_manager->get_width(comments[i]), cur_y - 24, comments[i], BROWSER, parent->texture_manager->get_height(comments[i]));
			auto expandable_ptr = expandable.get();
			auto expand_button = make_unique<Button>(300, cur_y - 24, 300 + 118, cur_y, "showcomments.png", BROWSER, [expandable_ptr, this, i]() 
			{
				expandable_ptr->toggle();
				if (i == 3)
				{
					this->parent->required_target = "reddit";
					this->parent->websites["reddit"] = { "sleepyred.png" };
				}
			}, []() {});
			expandable->never_focus = true;
			expandable->scroll_panel = this;
			children.push_back(expandable.get());
			children.push_back(expand_button.get());
			parent->to_be_added.push_back(std::move(expandable));
			parent->to_be_added.push_back(std::move(expand_button));
			auto line = make_unique<ScreenElement>(300, cur_y - 33, 700, cur_y - 32, "black.png", BROWSER);
			line->never_focus = true;
			children.push_back(line.get());
			parent->to_be_added.push_back(std::move(line));
			cur_y -= (inc_height + 50);
		}

		resize_page(-cur_y + 100);
		auto certify = make_unique<ScreenElement>(771, cur_y - 30, -1, -1, "certify.png", BROWSER);
		children.push_back(certify.get());
		certify->never_focus = true;
		parent->to_be_added.push_back(std::move(certify));
	}

	if (website_name == "game")
	{
		auto animation = make_unique<Animation>(660, 500 + 150, 660 + 600, 500 + 600 + 150, "scores", BROWSER, 13);
		auto animation_ptr = animation.get();
		animation->is_visible = false;
		children.push_back(animation.get());
		parent->to_be_added.push_back(std::move(animation));
		auto play = make_unique<Button>(300, 300, -1, -1, "startgame.png", BROWSER, []() {}, []() {}, "", "invisible.png", "gendark.png");
		auto scores = make_unique<Button>(1190, 300, -1, -1, "highscores.png", BROWSER, []() {}, []() {}, "", "invisible.png", "gendark.png");
		auto game = make_unique<BubbleGame>(0.0, 56.0, 1920.0, 1080 - 68, "black.png", parent, BROWSER);
		game->scroll_panel = this;
		auto returnbtn = make_unique<Button>(745, 100, -1, -1, "return.png", BROWSER, []() {}, []() {}, "", "invisible.png", "gendark.png");
		auto play_ptr = play.get();
		auto scores_ptr = scores.get();
		auto game_ptr = game.get();
		auto returnbtn_ptr = returnbtn.get();
		play->shiny = true;
		play->parent = parent;
		children.push_back(play.get());
		parent->to_be_added.push_back(std::move(play));
		scores->shiny = true;
		scores->parent = parent;
		children.push_back(scores.get());
		parent->to_be_added.push_back(std::move(scores));
		returnbtn->shiny = true;
		returnbtn->parent = parent;
		returnbtn->is_visible = false;
		children.push_back(returnbtn_ptr);
		parent->to_be_added.push_back(std::move(returnbtn));
		children.push_back(game.get());
		parent->to_be_added.push_back(std::move(game));
		scores_ptr->release_function = [game_ptr, play_ptr, scores_ptr, returnbtn_ptr, animation_ptr]()
		{
			game_ptr->score_mode = true;
			play_ptr->is_visible = false;
			scores_ptr->is_visible = false;
			returnbtn_ptr->is_visible = true;
			animation_ptr->is_visible = true;
		};

		returnbtn_ptr->release_function = [game_ptr, play_ptr, scores_ptr, returnbtn_ptr, animation_ptr]()
		{
			game_ptr->score_mode = false;
			play_ptr->is_visible = true;
			scores_ptr->is_visible = true;
			returnbtn_ptr->is_visible = false;
			animation_ptr->is_visible = false;
		};

		play_ptr->release_function = [game_ptr, play_ptr, scores_ptr, returnbtn_ptr]()
		{
			play_ptr->is_visible = false;
			scores_ptr->is_visible = false;
			game_ptr->initiate_game();
		};
	}

	if (website_name == "---")
	{
		auto mmo = make_unique<MMO>(0.0, 56.0, 1920.0, 1000.0, "black.png", "mmo.txt", parent, BROWSER);
		mmo->app_state = 1;
		mmo->chat_height = 0;
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

	if (website_name == "funcorner")
		generate_puzzle();

	if (website_name == "searchresults")
	{
		scroll_bar->is_visible = false;
		no_scroll = true;
		auto cover = make_unique<ScreenElement>(0, 0, 1920, 1080, "searchspinner.png", BROWSER);
		auto cover_ptr = cover.get();
		children.push_back(cover_ptr);
		auto spinner = make_unique<Spinner>(10, 0, glutGet(GLUT_WINDOW_WIDTH) / 2.0, glutGet(GLUT_WINDOW_HEIGHT) / 2.0, BROWSER);
		children.push_back(spinner.get());
		if (skip_loading)
		{
			spinner->frames = 200;
			spinner->is_visible = false;
			cover->is_visible = false;
		}

		spinner->drawn_by_browser = cover->drawn_by_browser = true;
		parent->to_be_added.push_back(std::move(spinner));
		parent->to_be_added.push_back(std::move(cover));
		animation_function = [this, cover_ptr, skip_loading]()
		{
			Spinner* spinner = NULL;
			if (this->children.size() > 1)
				spinner = dynamic_cast<Spinner*>(this->children[1]);

			if (spinner != NULL)
			{
				if (spinner->frames > 150 && white_cover_alpha.current_value == 0.0)
				{
					white_cover_alpha.step();
					white_cover_apex = [this, spinner, cover_ptr]()
					{
						spinner->marked_for_deletion = true;
						cover_ptr->marked_for_deletion = true;
						this->scroll_bar->is_visible = true;
						this->no_scroll = false;
						children.clear();

						auto sidebar = make_unique<ScreenElement>(0, 0, 500, 1080, "searchside.png", BROWSER);
						sidebar->no_translate = true;
						sidebar->never_focus = true;
						sidebar->browser_child = true;
						sidebar->drawn_by_browser = true;
						children.push_back(sidebar.get());
						parent->to_be_added.push_back(std::move(sidebar));

						if (!this->parent->answered_search_question)
						{
							auto question = make_unique<ScreenElement>(0, -820 + 56, 1920, -820 + 50 + 56, "didyoulike.png", BROWSER);
							question->browser_child = true;
							auto question_ptr = question.get();
							auto yes_button = make_unique<Button>(500, -820 + 56 + 5, -1, -1, "browseryes.png", BROWSER, []() {}, []() {}, "", "browseryesheld.png", "browseryespressed.png");
							auto no_button = make_unique<Button>(700, -820 + 56 + 5, -1, -1, "browserno.png", BROWSER, []() {}, []() {}, "", "browsernoheld.png", "browsernopressed.png");
							auto yes_ptr = yes_button.get();
							auto no_ptr = no_button.get();
							question_ptr->drawn_by_browser = yes_ptr->drawn_by_browser = no_ptr->drawn_by_browser = true;
							yes_button->release_function = [this, question_ptr, yes_ptr, no_ptr]()
							{
								question_ptr->name = "didyoulike3.png";
								yes_ptr->is_visible = false;
								no_ptr->is_visible = false;
								this->parent->answered_search_question = true;
							};
							no_button->release_function = [this, question_ptr, yes_ptr, no_ptr]()
							{
								question_ptr->name = "didyoulike2.png";
								yes_ptr->is_visible = false;
								no_ptr->is_visible = false;
								this->parent->answered_search_question = true;
							};
							yes_button->browser_child = true;
							no_button->browser_child = true;
							children.push_back(question.get());
							children.push_back(yes_ptr);
							children.push_back(no_ptr);
							parent->to_be_added.push_back(std::move(yes_button));
							parent->to_be_added.push_back(std::move(no_button));
							parent->to_be_added.push_back(std::move(question));
						}
						
						this->parent->process_control_changes();
					};
				}

				if (skip_loading)
				{
					white_cover_apex();
					white_cover_alpha.current_value = 0;
				}
			}
		};
	}

	if (website_name == "catalogue")
	{
		int cur_y = y2 - 20;
		for (int i = 0; i < shopping_center->inventory.size(); ++i)
		{
			auto item_background = make_unique<ScreenElement>(x1, cur_y - 236, x1 + 1400, cur_y, "invisible.png", VIDEO_APP);
			auto picture = make_unique<ScreenElement>(x1 + 10, cur_y - 200, x1 + 310 + 50, cur_y, shopping_center->inventory[i].image, VIDEO_APP);
			if (shopping_center->inventory[i].image == "baby")
				picture = make_unique<Video>(x1 + 10, cur_y - 300, x1 + 310, cur_y, "invisible.png", VIDEO_APP, parent);

			//auto name = make_unique<InfoText>(x1 + 550 - 10, cur_y - 100, x1 + 550 - 10, cur_y - 100, "invisible.png", shopping_center->inventory[i].name, VIDEO_APP);
			auto name = make_unique<StaticText>(x1 + 310 + 70, cur_y, shopping_center->inventory[i].name, VIDEO_APP, 64, parent);
			auto name_ptr = name.get();
			name->top_justify(cur_y);
			auto price = make_unique<InfoText>(x1 + 310 + 70, cur_y - 200, x1 + 310 + 70, cur_y - 200, "invisible.png", "$" + to_string(shopping_center->inventory[i].price), VIDEO_APP);
			auto black_line = make_unique<ScreenElement>(x1 + 10, cur_y - 230, x1 + 1286, cur_y - 220, "blackline.png", VIDEO_APP);
			auto property_text = make_unique<StaticText>(x1 + 500, cur_y - 100, "・Size: " + shopping_center->inventory[i].properties[0] + "\n・Durability: " + shopping_center->inventory[i].properties[1] + "\n・Educational: " + shopping_center->inventory[i].properties[2] + "\n・Elligible for refund: " + shopping_center->inventory[i].properties[3], VIDEO_APP, 32, parent, true);
			picture->browser_child = name->browser_child = price->browser_child = black_line->browser_child = property_text->browser_child = item_background->browser_child = true;
			picture->drawn_by_browser = name->drawn_by_browser = price->drawn_by_browser = black_line->drawn_by_browser = property_text->drawn_by_browser = item_background->drawn_by_browser = true;
			children.push_back(item_background.get());
			children.push_back(picture.get());
			children.push_back(name.get());
			children.push_back(price.get());
			children.push_back(black_line.get());
			children.push_back(property_text.get());
			parent->to_be_added.push_back(std::move(picture));
			parent->to_be_added.push_back(std::move(name));
			parent->to_be_added.push_back(std::move(price));
			parent->to_be_added.push_back(std::move(black_line));
			parent->to_be_added.push_back(std::move(property_text));
			parent->to_be_added.push_back(std::move(item_background));
			if (shopping_center->item_in_cart(shopping_center->inventory[i].name) || shopping_center->is_purchased(shopping_center->inventory[i].name))
			{
				auto black = make_unique<ScreenElement>(x1 + 10, cur_y - 320, x1 + 510, cur_y, "black.png", VIDEO_APP);
				black->alpha = 0.5;
				black->drawn_by_browser = true;
				children.push_back(black.get());
				parent->to_be_added.push_back(std::move(black));
			}

			else links.push_back(Link(Rect(name_ptr->x1, name_ptr->y1, name_ptr->x2, name_ptr->y2), "itemstore|" + shopping_center->inventory[i].name));
			cur_y -= 360;
		}

		resize_page(-cur_y - y1);
		never_focus_children();
	}

	if (website_name == "ownership")
	{
		int cur_y = y2 - 20;
		if (!shopping_center->visited_ownership)
		{
			auto welcome = make_unique<ScreenElement>(x1 + 10, y2 - 20, -1, -1, "ownershipwelcome.png", VIDEO_APP);
			children.push_back(welcome.get());
			parent->to_be_added.push_back(std::move(welcome));
			cur_y -= 20;
		}

		for (int i = 0; i < shopping_center->purchased.size(); ++i)
		{
			auto picture = make_unique<ScreenElement>(x1 + 10, cur_y - 300, x1 + 310, cur_y, shopping_center->purchased[i].image, VIDEO_APP);
			if (shopping_center->purchased[i].image == "baby")
				picture = make_unique<Video>(x1 + 10, cur_y - 300, x1 + 310, cur_y, "invisible.png", VIDEO_APP, parent);

			auto name = make_unique<InfoText>(x1 + 350 - 10, cur_y - 100, x1 + 350 - 10, cur_y - 100, "invisible.png", shopping_center->purchased[i].name, VIDEO_APP);
			auto progress = make_unique<ProgressBar>(x1 + 350 - 10, cur_y - 200, x1 + 350 + 100, cur_y - 150, "shinyred.png", parent, VIDEO_APP, []() {}, true);
			progress->progress = shopping_center->purchased[i].ownership;
			progress->do_animate = false;
			auto black_line = make_unique<ScreenElement>(x1 + 10, cur_y - 330, x1 + 510, cur_y - 320, "blackline.png", VIDEO_APP);
			picture->browser_child = name->browser_child = progress->browser_child = black_line->browser_child = true;
			picture->drawn_by_browser = name->drawn_by_browser = progress->drawn_by_browser = black_line->drawn_by_browser = true;
			children.push_back(picture.get());
			children.push_back(name.get());
			children.push_back(progress.get());
			children.push_back(black_line.get());
			parent->to_be_added.push_back(std::move(picture));
			parent->to_be_added.push_back(std::move(name));
			parent->to_be_added.push_back(std::move(progress));
			parent->to_be_added.push_back(std::move(black_line));
			cur_y -= 360;
		}

		resize_page(-cur_y - y1);
		if (!scroll_bar->is_visible)
			no_scroll = true;
	}

	if (website_name == "messages")
	{
		int cur_y = y2 - 20;
		for (int i = 0; i < shopping_center->message_queue.size(); ++i)
		{
			string cur_name = shopping_center->message_queue[i];
			int width = parent->texture_manager->get_width(cur_name);
			int height = parent->texture_manager->get_height(cur_name);
			auto message = make_unique<ScreenElement>(x1 + 10, cur_y - height, x1 + 10 + width, cur_y, cur_name, VIDEO_APP);
			message->browser_child = message->drawn_by_browser = true;
			children.push_back(message.get());
			parent->to_be_added.push_back(std::move(message));
			auto line = make_unique <ScreenElement>(x1 + 10, cur_y - height - 10, x2 - 10, cur_y - height - 9, "black.png", VIDEO_APP);
			line->browser_child = line->drawn_by_browser = true;
			children.push_back(line.get());
			parent->to_be_added.push_back(std::move(line));
			cur_y -= (height + 20);
		}

		resize_page(-cur_y - y1);
		if (!scroll_bar->is_visible)
			no_scroll = true;
	}

	if (website_name == "cart")
	{
		int cur_y = y2 - 50;
		auto total_message = make_unique<InfoText>(x1 + 10, y2 - 30, x1 + 10, y2 - 30, "invisible.png", "Total: $" + shopping_center->two_decimals(shopping_center->get_total()), VIDEO_APP);
		total_message->browser_child = total_message->drawn_by_browser = true;
		children.push_back(total_message.get());
		parent->to_be_added.push_back(std::move(total_message));
		for (int i = 0; i < shopping_center->cart.size(); ++i)
		{
			auto picture = make_unique<ScreenElement>(x1 + 10, cur_y - 300, x1 + 310, cur_y, shopping_center->cart[i].image, VIDEO_APP);
			if (shopping_center->cart[i].image == "baby")
				picture = make_unique<Video>(x1 + 10, cur_y - 300, x1 + 310, cur_y, "invisible.png", VIDEO_APP, parent);

			auto name = make_unique<InfoText>(x1 + 350 - 10, cur_y - 100, x1 + 350 - 10, cur_y - 100, "invisible.png", shopping_center->cart[i].name, VIDEO_APP);
			auto remove = make_unique<InfoText>(x1 + 350 - 10, cur_y - 150, x1 + 350 - 10, cur_y - 150, "invisible.png", "Remove from Cart", VIDEO_APP);
			auto price = make_unique<InfoText>(x1 + 350 - 10, cur_y - 200, x1 + 350 - 10, cur_y - 200, "invisible.png", to_string(shopping_center->cart[i].price), VIDEO_APP);
			auto black_line = make_unique<ScreenElement>(x1 + 10, cur_y - 330, x1 + 510, cur_y - 320, "blackline.png", VIDEO_APP);
			remove->browser_child = picture->browser_child = name->browser_child = price->browser_child = black_line->browser_child = true;
			remove->drawn_by_browser = picture->drawn_by_browser = name->drawn_by_browser = price->drawn_by_browser = black_line->drawn_by_browser = true;
			children.push_back(picture.get());
			children.push_back(name.get());
			children.push_back(price.get());
			children.push_back(black_line.get());
			children.push_back(remove.get());
			parent->to_be_added.push_back(std::move(picture));
			parent->to_be_added.push_back(std::move(name));
			parent->to_be_added.push_back(std::move(price));
			parent->to_be_added.push_back(std::move(black_line));
			parent->to_be_added.push_back(std::move(remove));
			Link remove_link = Link(Rect(x1 + 350, cur_y - 150, x1 + 350 + 50, cur_y - 100), "cart");
			string item_name = shopping_center->cart[i].name;
			remove_link.link_action = [this, item_name]() 
			{
				this->shopping_center->remove_item_from_cart(item_name);
				this->change_website("catalogue");
				this->parent->process_control_changes();
				this->change_website("cart");
			};

			links.push_back(remove_link);
			cur_y -= 360;
		}

		resize_page(-cur_y - y1);
		if (!scroll_bar->is_visible)
			no_scroll = true;
	}

	if (website_name == "itemstore")
	{
		scroll_bar->is_visible = false;
		no_scroll = true;
		int cur_y = y2 - 20;
		ShoppingItem current_item = ShoppingItem("", "", 0, {});
		for (int i = 0; i < shopping_center->inventory.size(); ++i)
		{
			if (shopping_center->inventory[i].name == shopping_center->current_item)
				current_item = shopping_center->inventory[i];
		}

		auto picture = make_unique<ScreenElement>(x1 + 10, cur_y - 300, x1 + 310, cur_y, current_item.image, VIDEO_APP);
		if (current_item.image == "baby")
			picture = make_unique<Video>(x1 + 10, cur_y - 300, x1 + 310, cur_y, "invisible.png", VIDEO_APP, parent);

		auto name = make_unique<InfoText>(x1 + 350 - 10, cur_y - 100, x1 + 350 - 10, cur_y - 100, "invisible.png", current_item.name, VIDEO_APP);
		auto price = make_unique<InfoText>(x1 + 350 - 10, cur_y - 200, x1 + 350 - 10, cur_y - 200, "invisible.png", to_string(current_item.price), VIDEO_APP);
		auto add_to_cart = make_unique<InfoText>(x1 + 10, cur_y - 400, x1 + 10, cur_y - 400, "invisible.png", "Add to Cart", VIDEO_APP);
		auto back_to_catalogue = make_unique<InfoText>(x1 + 10, cur_y - 500, x1 + 10, cur_y - 500, "invisible.png", "Back to Catalogue", VIDEO_APP);
		links.push_back(Link(Rect(x1 + 10, cur_y - 500, x1 + 110, cur_y - 450), "catalogue"));
		Link cart_action = Link(Rect(x1 + 10, cur_y - 400, x1 + 110, cur_y - 350), "catalogue");
		cart_action.link_action = [this, current_item]() {this->shopping_center->cart.push_back(current_item);};
		links.push_back(cart_action);
		children.push_back(picture.get());
		children.push_back(name.get());
		children.push_back(price.get());
		children.push_back(add_to_cart.get());
		children.push_back(back_to_catalogue.get());
		parent->to_be_added.push_back(std::move(picture));
		parent->to_be_added.push_back(std::move(name));
		parent->to_be_added.push_back(std::move(price));
		parent->to_be_added.push_back(std::move(add_to_cart));
		parent->to_be_added.push_back(std::move(back_to_catalogue));
	}

	for (int i = 0; i < children.size(); ++i)
		children[i]->browser_child = true;
}

int ScrollPanel::create_radio_button_group(int start_x, int start_y, int number, bool horizontal, vector<string> labels, int change)
{
	if (!horizontal)
	{
		auto label = make_unique<CachedText>(start_x, start_y + 60, start_x, start_y + 40, "white.png", labels[0], BROWSER);
		children.push_back(label.get());
		parent->to_be_added.push_back(std::move(label));
	}

	int x_offset = 0, y_offset = 0;
	if (horizontal)
		x_offset += 140;

	auto master_button = make_unique<RadioButton>(start_x + x_offset, start_y, start_x + 15 + x_offset, start_y + 15, "radiobutton.png", parent, BROWSER, (RadioButton*) NULL, this);
	//master_button->never_focus = true;
	auto master_label = make_unique<CachedText>(start_x + 25, start_y + 5, start_x + 25, start_y + 5, "white.png", labels[1], BROWSER);
	if (master_label->text != "")
		master_label->center_on(start_y + 7, parent->texture_manager);

	children.push_back(master_button.get());
	children.push_back(master_label.get());
	number--;
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
		auto child_button = make_unique<RadioButton>(start_x + x_offset, start_y + y_offset, start_x + x_offset + 15, start_y + y_offset + 15, "radiobutton.png", parent, BROWSER, master_button.get(), this);
		//child_button->never_focus = true;
		master_button->children.push_back(child_button.get());
		children.push_back(child_button.get());
		parent->to_be_added.push_back(std::move(child_button));
		if (!horizontal)
		{
			auto label = make_unique<CachedText>(start_x + x_offset + 25, start_y + y_offset + 5, start_x + x_offset + 75, start_y + y_offset + 5, "invisible.png", labels[i + 2], BROWSER);
			if (label->text != "")
				label->center_on(start_y + y_offset + 7, parent->texture_manager);

			children.push_back(label.get());
			parent->to_be_added.push_back(std::move(label));
		}
	}

	parent->to_be_added.push_back(std::move(master_button));
	parent->to_be_added.push_back(std::move(master_label));
	return start_y + y_offset;
}

void ScrollPanel::create_survey_submit_button(int x, int y, function<void()> on_success, function<void()> on_failure, string name1, string name2, string name3)
{
	auto button = make_unique<Button>(x, y, x + parent->texture_manager->get_width(name1), y + parent->texture_manager->get_height(name1), name1, BROWSER,
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
	}, [](){}, "", name2, name3);
	children.push_back(button.get());
	parent->to_be_added.push_back(std::move(button));
}

void ScrollPanel::generate_puzzle()
{
	vector<int> numbers = { 7, 8, 0, 4, 5, 6, 1, 2, 3 };
	int index = 0;
	int cur_x = 600;
	int cur_y = 1080 - 575;
	vector<vector<int>> board;
	for (int i = 0; i < 3; ++i)
	{
		vector<int> row;
		for (int j = 0; j < 3; ++j)
		{
			int num = numbers[index];
			if (num != 0)
			{
				string tile_name = "tile";
				tile_name += to_string(num);
				tile_name += ".png";
				auto puzzle_piece = make_unique<SlidingTile>(cur_x, cur_y, cur_x + 75, cur_y + 75, tile_name, num, j, 2 - i, BROWSER);
				children.push_back(puzzle_piece.get());
				parent->to_be_added.push_back(std::move(puzzle_piece));
			}

			cur_x += 80;
			row.push_back(num);
			index++;
		}

		cur_y += 80;
		cur_x = 600;
		board.insert(board.begin(), row);
	}

	auto win_label = make_unique<ScreenElement>(600, 400, 664, 413, "youwin.png", BROWSER);
	win_label->is_visible = false;
	SlidingTile::win_label = win_label.get();
	SlidingTile::board = board;
	SlidingTile::win_locked = true;
	for (int i = 0; i < 100; ++i)
		children[rand() % 8]->mouse_clicked(GLUT_LEFT, GLUT_DOWN, 0, 0);

	SlidingTile::win_locked = false;
	children.push_back(win_label.get());
	parent->to_be_added.push_back(std::move(win_label));
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
	if (!retain_white_cover_alpha)
		white_cover_alpha.restart();

	white_cover_apex = []() {};
}

void ScrollPanel::start_loading_site(string website_name, bool force_through)
{
	site_change_frame = 0;
	will_force_through = false;
	currently_loading_site = true;
	if (application != VIDEO_APP)
		parent->to_be_added.push_back(make_unique<LoadingIndicator>(this));

	site_to_be_loaded = website_name;
	loading_time = rand() % 1000;
	if (loading_time == 0)
		loading_time = 60;

	if (application == VIDEO_APP)
		loading_time = 0;

	if (site_to_be_loaded == "thankyouforuploading" && prev_website != "thankyouforuploading")
		loading_time = 5000;

	time_started_loading = glutGet(GLUT_ELAPSED_TIME);
}

void ScrollPanel::draw(TextureManager* texture_manager)
{
	if (application != BROWSER) //TODO if browser actually needs this then revisit search page text field
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
	}

    float old_offset = offset;
	if (offset > sum_of_heights - (y2 - y1))
	{
		offset = sum_of_heights - (y2 - y1);
		move_links(offset - old_offset);
	}

	int current_y = this->y2 + round(offset);
	bool increase_last_height = false;
	int real_sum_of_heights = 0;
	for (int i = 0; i < textures.size(); ++i)
		real_sum_of_heights += texture_manager->get_height(textures[i]);

	if (real_sum_of_heights < sum_of_heights)
		increase_last_height = true;

	if (prev_website == "forumhome" || prev_website == "forumthreads" || prev_website == "thread11" || prev_website == "thread12" || prev_website == "thread21" || prev_website == "thread22" || prev_website == "thread31" || prev_website == "thread32" || prev_website == "thread41" || prev_website == "thread51" || prev_website == "thread52")
	{
		ScreenElement blue_background(0, 0, 1920, 1080, "bluebackground.png");
		blue_background.draw(texture_manager);
	}
	
	if (!parent->done_cracking || !parent->everything_stuck)
	{
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
	}

	else
	{
		ScreenElement black = ScreenElement(0, 0, 1920, 1080, "black.png");
		black.draw(texture_manager);
		for (int i = 0; i < glass_textures.size(); ++i)
			glass_textures[i].draw(texture_manager);
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

	if (parent->everything_stuck && !parent->done_cracking)
	{
		for (int i = 0; i < break_points.size(); ++i)
		{
			int center_x = break_points[i].first;
			int center_y = break_points[i].second;
			ScreenElement glass(center_x - big_glass_width / 2.0, center_y - big_glass_height / 2.0, center_x + big_glass_width / 2.0, center_y + big_glass_height / 2.0, "bigglass.png");
			//glColor4f(1.0, 1.0, 1.0, 0.27);
			glass.draw(texture_manager);
			glColor4f(1.0, 1.0, 1.0, 1.0);
		}

		if (break_points.size() >= num_break_points)
			drawn_all_glass = true;
	}

	for (int i = 0; i < children.size(); ++i)
	{
		if (children[i]->drawn_by_browser && children[i]->is_visible)
			children[i]->draw(texture_manager);
	}

	if (application != BROWSER)
	{
		glStencilMask(~0);
		glClear(GL_STENCIL_BUFFER_BIT);
		glDisable(GL_STENCIL_TEST);
	}

	if (white_cover_alpha.current_value != 0.0)
	{
		glDisable(GL_TEXTURE_2D);
		glColor4f(1.0, 1.0, 1.0, white_cover_alpha.current_value);
		glRectf(x1, y1, x2, y2);
		glColor4f(1.0, 1.0, 1.0, 1.0);
		glEnable(GL_TEXTURE_2D);
	}

	if (scroll_locked)
	{
		parent->draw_arrow_thing = true;
		parent->scroll_lock_x = scroll_lock_x;
		parent->scroll_lock_y = scroll_lock_y;
	}

	if (parent->draw_underline)
	{
		glColor4f(underline_red, underline_green, underline_blue, 1.0);
		ScreenElement line(underlinex1, underliney, underlinex2, underliney + 1, "white.png");
		line.draw(texture_manager);
		glColor4f(1.0, 1.0, 1.0, 1.0);
	}
}

void ScrollPanel::press_key(unsigned char key)
{
	if (scroll_locked)
		return;

	if ((key == KEY_UP || key == KEY_DOWN) && (browser_key.key != key || browser_key.death_counter != -1))
		browser_key = BrowserKey(key, -1);
}

/*void ScrollPanel::press_key_backing(unsigned char key, int mode)
{
	cout << "press key " << key << endl;
	if (parent->everything_stuck || no_scroll)
		return;

	if (mode == 1)
	{
		if (key == KEY_UP)
			animating_scroll = 1;

		else if (key == KEY_DOWN)
			animating_scroll = 2;

		else return;
		frame_started_scroll = myframes;
		return;
	}

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
            scroll_bar->move((scroll_bar->range() * scroll_speed) / sum_of_heights);
            break;
            
        case KEY_DOWN:
            if (!going_up && timebase != -1)
                return;
            
            going_up = false;
            offset += scroll_speed;
			if (offset > sum_of_heights - (y2 - y1))
				offset = sum_of_heights - (y2 - y1);
            scroll_bar->move((scroll_bar->range() * -scroll_speed) / sum_of_heights);
            break;
            
        default:
            return;
    }
    
    timebase = glutGet(GLUT_ELAPSED_TIME);
    move_links(offset - old_offset);
    update_cursor(prev_mouse_x, prev_mouse_y);
	if (offset != old_offset)
		parent->mouse_moved(parent->prev_mouse_x, 1080 - parent->prev_mouse_y, this);
}*/

void ScrollPanel::scroll_page(int speed)
{
	if (parent->everything_stuck || no_scroll)
		return;

	float old_offset = offset;
	offset += speed;
	if (offset < 0)
		offset = 0;

	if (offset > sum_of_heights - (y2 - y1))
		offset = sum_of_heights - (y2 - y1);

	scroll_bar->move((scroll_bar->range() * -speed) / sum_of_heights);
	move_links(offset - old_offset);
	update_cursor(prev_mouse_x, prev_mouse_y);
	if (offset != old_offset)
		parent->mouse_moved(parent->prev_mouse_x, 1080 - parent->prev_mouse_y, this, true);
}

void ScrollPanel::release_key(unsigned char key)
{
	if (browser_key.key == key)
		browser_key.death_counter = 10;
}

void ScrollPanel::animate()
{
	myframes++;
	if (browser_key.key != -1)
	{
		if (browser_key.key == KEY_UP || browser_key.key == KEY_DOWN)
		{
			if (browser_key.frames <= 8)
				browser_key.velocity = 7;

			else if (browser_key.frames < 30)
				browser_key.velocity = 0;

			else if (browser_key.frames == 30)
			{
				if (browser_key.death_counter == -1)
					browser_key.velocity = 20;
			}

			else
			{
				if (browser_key.velocity != 0)
				{
					if (browser_key.death_counter != -1)
						browser_key.velocity = browser_key.death_counter;
				}
			}
		}

		if (browser_key.key == 3 || browser_key.key == 4)
		{
			if (browser_key.death_counter >= 7)
			{
				if (browser_key.velocity < 40)
				{
					browser_key.velocity += 5;
					if (browser_key.velocity > 40)
						browser_key.velocity = 40;
				}
			}

			if (browser_key.death_counter <= 4)
				browser_key.velocity = round((float)browser_key.velocity * 0.5);
		}

		browser_key.frames++;
		if (browser_key.death_counter != -1)
			browser_key.death_counter--;

		scroll_page((browser_key.key == 3 || browser_key.key == KEY_UP) ? -browser_key.velocity : browser_key.velocity);
		if (browser_key.death_counter == 0)
			browser_key = BrowserKey();
	}

	if (white_cover_alpha.current_value > 0.0)
	{
		white_cover_alpha.step();
		if (white_cover_alpha.current_value >= 1.0)
			white_cover_apex();
	}

	else white_cover_alpha.restart();
	
	if (drawn_all_glass)
	{
		auto pieces = get_pieces(1920, 1000 - 56, 0, 56, "bigglassmap.png", big_glass_width, big_glass_height);
		create_glass_textures(pieces, 1920, 1080, 0, 56, "search.png", "bigglass.png", big_glass_width, big_glass_height, "");
		parent->done_cracking = true;
		drawn_all_glass = false;
	}

	if (parent->everything_stuck)
	{
		bool found = false;
		for (int i = 0; i < glass_textures.size(); ++i)
		{
			glass_textures[i].animate();
			if (glass_textures[i].y2 > -1000)
				found = true;
		}

		if (!found && glass_textures.size() != 0)
			parent->restore_browser();
	}

	animation_function();
	if (currently_loading_site)
	{
		site_change_frame++;
		int cur_time = glutGet(GLUT_ELAPSED_TIME);
		if (cur_time - time_started_loading >= loading_time)
		{
			currently_loading_site = false;
			change_website(site_to_be_loaded, true, will_force_through);
			if (site_to_be_loaded != "thankyouforuploading")
				will_force_through = false;

			parent->process_control_changes();
		}
	}

	if (scroll_lock_speed != 0)
		scroll_page(scroll_lock_speed);
}

//TODO forum page buttons are stretched by a few pixels for some reason
void ScrollPanel::add_link_button(vector<string> components, string website_name)
{
	string button_name = components[4].substr(1, components[4].length());
	auto name_components = split(button_name, '_');
	if (name_components[0] == "forumthreads")
	{
		auto button = make_unique<Button>(atof(components[0].c_str()), atof(components[1].c_str()), -1, -1, "page" + name_components[1] + "light.png",
			application, [this]()
		{
			this->start_loading_site("forumhome");
			this->will_force_through = true;
		}, []() {}, "", "page" + name_components[1] + "dark.png", "page" + name_components[1] + "dark.png");
		button->mouse_over_function = [this]()
		{
			this->parent->set_cursor("cursorhand.png");
			this->parent->someone_set_cursor = true;
		};
		children.push_back(button.get());
		parent->to_be_added.push_back(std::move(button));
	}

	else if (name_components[0] == "page")
	{
		string start = website_name.substr(0, website_name.size() - 1);
		string end = website_name.substr(website_name.size() - 1, 1);
		string lightdark = "";
		if (end == name_components[1])
			lightdark = "dark.png";

		else lightdark = "light.png";
		auto button = make_unique<Button>(atof(components[0].c_str()), atof(components[1].c_str()), -1, -1, "page" + name_components[1] + lightdark, application, [this, start, name_components]()
		{
			this->start_loading_site(start + name_components[1]);
			this->will_force_through = true;
		}, []() {}, "", "page" + name_components[1] + "dark.png", "page" + name_components[1] + "dark.png");
		button->mouse_over_function = [this]()
		{
			this->parent->set_cursor("cursorhand.png");
			this->parent->someone_set_cursor = true;
		};
		children.push_back(button.get());
		parent->to_be_added.push_back(std::move(button));
	}
}

void ScrollPanel::make_popup(string text)
{
	no_scroll = true;
	auto grey = make_unique<ScreenElement>(0, 56, 1920, 1012, "greyout.png", BROWSER);
	auto info_text = make_unique<InfoText>(560.0, 470.0, 560 + 360, 470 + 200, "browserdialog.png", text, BROWSER);
	auto close_button = make_unique<Button>(660, 475, 660 + 150, 475 + 40, "browserok.png", BROWSER, []() {}, []() {}, "", "browserheld.png", "browserpressed.png");
	auto grey_ptr = grey.get();
	auto info_text_ptr = info_text.get();
	auto close_button_ptr = close_button.get();
	info_text->center_horiz = true;
	close_button_ptr->release_function = [grey_ptr, info_text_ptr, close_button_ptr, this]()
	{
		no_scroll = false;
		grey_ptr->marked_for_deletion = info_text_ptr->marked_for_deletion = close_button_ptr->marked_for_deletion = true;
		this->children.erase(children.end() - 3, children.end());
		this->parent->process_control_changes();
	};

	children.push_back(grey_ptr);
	children.push_back(info_text_ptr);
	children.push_back(close_button_ptr);
	parent->to_be_added.push_back(std::move(grey));
	parent->to_be_added.push_back(std::move(info_text));
	parent->to_be_added.push_back(std::move(close_button));
	auto deltas = parent->to_be_added[parent->to_be_added.size() - 2]->center();
	parent->to_be_added[parent->to_be_added.size() - 1]->translate(deltas.first, deltas.second);
}

void ScrollPanel::change_website(std::string website_name, bool new_branch, bool force_through, bool skip_loading)
{
	if (parent->required_target != "")
	{
		website_name = parent->required_target;
		parent->required_target = "";
	}

	parent->stop_website_sounds();
	if (parent->everything_stuck)
		return;

	if (parent->reached_help_center && !parent->reached_second_day)
	{
		parent->delist("helphome");
		parent->delist("searchresults");
		parent->websites["ain"] = { "ain2.png" };
		//parent->websites["useful"] = { "useful20.png" };
		parent->delist("reddit");
		parent->delist("tablet");
		parent->delist("conversation");
		parent->delist("tomthread");
		parent->sequences["sequenceain"] = { "psychologist" };
		parent->whitelist.push_back("psychologist");
		parent->submitted_poll_of_the_day = false;
		parent->reached_second_day = true;
		parent->day_number++;
		save_game(parent->save_slot);
	}
	
	trying_to_load = "";
	no_scroll = false;
	string orig_name = website_name;
	string search_string = "";
	if (website_name.size() >= 16 && website_name.substr(0, 16) == "search/search?q=")
	{
		website_name = "search/search?q=";
		search_string = orig_name.substr(16, string::npos);
	}

	if (website_name == prev_website && !(prev_website == "error" && error_display != history.back().second) && prev_website != "nenda")
		return;
	
	if (!whitelist_check(website_name, force_through) && website_name != "error")
	{
		change_website("error", true, true);
		if (error_display != "")
		{
			text_field_ptr->reset();
			text_field_ptr->text = error_display;
		}

		return;
	}

	if (website_name == "browse")
	{
		parent->start_application(FILES, "", true);
		parent->restore_application = BROWSER;
		auto files = make_unique<Files>(770.0 - 300, 404.0 - 60, 1150.0, 744.0 - 60, "white.png", parent, INFO_BOX);
		auto files_ptr = files.get();
		auto button = make_unique<Button>(955.0, /*409.0*/394 - 120 + 8 + 10, 1145.0, /*459.0*/394 - 120 + 8 + 60 - 10, "ok.png", INFO_BOX,
			[this, files_ptr]()
		{
			if (files_ptr->name_field_ptr->text == "")
			{
				this->parent->start_application(INFO_BOX, "Must specify a file name."); //TODO revisit this when fix bug about one info box closing all info boxes
				return;
			}

			files_ptr->file_open_function(files_ptr->name_field_ptr->text);
			this->parent->close_application(INFO_BOX);
		}, []() {});
		auto name_box = make_unique<TextField>(945 - 400, 394 - 120 + 8 + 10, 945, 394 - 120 + 8 + 60 - 10, "systemtextfield.png", parent, INFO_BOX);
		files_ptr->name_field_ptr = name_box.get();
		parent->to_be_added.insert(parent->to_be_added.end() - 1, std::move(files));
		files_ptr->enter_directory(0);
		files_ptr->file_open_function = [this, files_ptr](string file_name)
		{
			selection_field_ptr->reset();
			selection_field_ptr->text = ((files_ptr->folder_index == 0) ? "" : parent->get_cur_dir_name(files_ptr->folder_index)) + "/" + file_name;
			this->parent->close_application(INFO_BOX);
		};
		parent->to_be_added.push_back(std::move(name_box));
		parent->to_be_added.push_back(std::move(button));
		parent->to_be_added.push_back(make_unique<InfoText>(770 - 300, 394 - 120 + 8 + 10 + 15, 770 - 300, 394 - 120 + 8 + 10 + 15, "invisible.png", "Name:"));
		parent->tag_info_box();
		return;
	}

	if (website_name == "upload")
	{
		string choice = selection_field_ptr->text;
		if (choice[0] != '/')
		{
			make_popup("File not found.");
			return;
		}

		auto components = real_split(choice, '/');
		bool success = false;
		if (components.size() == 2)
			success = this->upload_image_file(components[1], "/");

		else if (components.size() > 2)
			success = this->upload_image_file(components[components.size() - 1], components[components.size() - 2]);

		if (success)
		{
			this->start_loading_site("thankyouforuploading");
			this->will_force_through = true;
		}

		else
		{
			make_popup("File not found.");
		}
		
		return;
	}

	animation_function = []() {};
	extend = false;
	if (website_name == "e2")
		parent->forcing_e2 = true;

	if (website_name == "vote")
	{
		make_popup("You must be logged in to perform that function.");
		return;
	}

	if (website_name == "break")
	{
		parent->make_everything_stuck();
	}
	
	if (website_name == "paint")
	{
		parent->start_application(INFO_BOX, "", false, true);
		parent->restore_application = BROWSER;
		return;
	}

	if (website_name == "download_trial")
	{
		parent->start_application(INFO_BOX, "", false, true, 1);
		parent->restore_application = BROWSER;
		return;
	}

	if (website_name == "decrypt")
	{
		parent->start_application(INFO_BOX, "", false, true, 2);
		parent->restore_application = BROWSER;
		return;
	}

	if (website_name == "mmo")
	{
		parent->start_application(INFO_BOX, "", false, true, 3);
		return;
	}

	if (website_name == "compiler")
	{
		parent->start_application(INFO_BOX, "", false, true, 4);
		return;
	}

	if (website_name == "lesson1code")
	{
		parent->start_application(INFO_BOX, "", false, true, 5);
		return;
	}

	if (website_name == "lesson2code")
	{
		parent->start_application(INFO_BOX, "", false, true, 6);
		return;
	}

	if (website_name == "lesson3code")
	{
		parent->start_application(INFO_BOX, "", false, true, 7);
		return;
	}

	if (website_name == "walter1")
	{
		parent->whitelist.push_back("paint");
		parent->whitelist.push_back("contestupload");
	}

	if (website_name == "arcade")
	{
		parent->whitelist.push_back("linesexist");
		parent->whitelist.push_back("squaresexist");
		parent->whitelist.push_back("brandonexist");
		parent->whitelist.push_back("existconquest");
		parent->whitelist.push_back("deliveryexist");
		parent->whitelist.push_back("typingexist");
	}

	if (website_name == "existconquest")
		parent->whitelist.push_back("game");

	if (website_name == "game")
	{
		bool do_save = false;
		if (!parent->reached_space_game)
			do_save = true;

		parent->reached_space_game = true;
		if (do_save)
			save_game(parent->save_slot);
	}

	if (website_name == "decryptthread")
		parent->whitelist.push_back("decryptpage");

	if (website_name.size() > 9 && website_name.substr(0, 9) == "itemstore")
	{
		auto strs = split(website_name, '|');
		shopping_center->current_item = strs[1];
		change_website("itemstore");
		return;
	}

	auto strings = split(website_name, '/');
	string display_name = website_names[strings[0]];
	if (strings[1] != "q")
		display_name += ("/" + strings[1]);

	if (parent->websites.find(website_name) == parent->websites.end())
	{
		text_field_ptr->reset();
		text_field_ptr->text = website_name;
		error_display = website_name;
		if (website_names[website_name] != "")
		{
			text_field_ptr->text = website_names[website_name];
			error_display = website_names[website_name];
		}

		if (special_error_display[website_name] != "")
		{
			text_field_ptr->text = special_error_display[website_name];
			error_display = special_error_display[website_name];
		}

		change_website("error", new_branch);
		return;
	}

	bool same_site = (prev_website == website_name);
	prev_website = website_name;
	//TODO ORIGINAL LOCATION OF MARKING VISITED_SITE
	if (website_name != "error")
	{
		text_field_ptr->reset();
		text_field_ptr->text = display_name + search_string;
	}

	if (parent->read_email["rosecolored1bold.png"] && (website_name == "search" || website_name == "reddit" || website_name == "useful")) //TODO doesn't check enough pages
	{
		if (!parent->visited_elsewhere_after_rosecolored1)
			parent->email_counter = 1;

		parent->visited_elsewhere_after_rosecolored1 = true;
	}

	if (parent->visited_site["search"] && parent->visited_site["reddit"] && parent->visited_site["tablet"] && parent->visited_site["useful"] && parent->visited_site["nenda"] &&
		parent->visited_site["rfh1"] && parent->visited_site["rfh2"] && parent->visited_site["forumemail1"] && parent->visited_site["forumemail2"] && parent->visited_site["deliverfailure"] &&
		parent->visited_site["ain"] && parent->visited_site["e1"] && parent->visited_site["conversation"] && parent->visited_site["tomthread"] && (website_name == "search" || website_name == "reddit" || website_name == "useful"))
		parent->visited_elsewhere_before_rosecolored1 = true;
	
	if (new_branch && !same_site)
	{
		string second = "";
		if (website_name == "error")
			second = error_display;

		if (!history.empty())
			history.erase(history.begin() + history_index + 1, history.end());

		if (website_name == "search/search?q=")
			history.push_back(pair<string, string>(orig_name, second));

		else history.push_back(pair<string, string>(website_name, second));
		history_index++;
	}
    
	offset = 0;
	textures = parent->websites[website_name];
    sum_of_heights = 0; //TODO used to be 56
	if (/*textures.size() == 1*/ parent->texture_manager->get_height(textures[0]) <= 1080.0 && textures[0] != "eula.png")
	{
		sum_of_heights = 0;
		scroll_bar->is_visible = false;
	}

	else
		scroll_bar->is_visible = true;

	for (int i = 0; i < textures.size(); ++i)
		resize_page(parent->texture_manager->get_height(textures[i]));

	scroll_bar->translate(0, scroll_bar->max_y - scroll_bar->y2);
	links.clear();
	remove_children();
	for_each(textures.begin(), textures.end(), [this, website_name](string& texture)
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
			if (line == "nocursor")
			{
				links.back().no_cursor = true;
				continue;
			}

			bool underline;
			float underline_red, underline_green, underline_blue;
			underline_red = underline_green = underline_blue = 0;
			if (components.size() >= 6 && (components[5] == "u" || components[5] == "=" || components[5] == "[" || components[5] == "]" || components[5] == "&"))
			{
				underline = true;
				if (components[5] == "=")
				{
					underline_red = 164.0 / 255.0;
					underline_green = 49.0 / 255.0;
					underline_blue = 28.0 / 255.0;
				}

				if (components[5] == "[")
				{
					underline_red = 2.0 / 255.0;
					underline_green = 115.0 / 255.0;
					underline_blue = 2.0 / 255.0;
				}

				if (components[5] == "]")
				{
					underline_red = 1.0;
					underline_green = 126.0 / 255.0;
					underline_blue = 0.0;
				}

				if (components[5] == "&")
				{
					underline_red = 1.0;
					underline_green = 1.0;
					underline_blue = 1.0;
				}
			}

			else underline = false;
			int underline_offset = (components.size() >= 7) ? atoi(components[6].c_str()) : 0;
			if (components[4][0] == '!')
				add_link_button(components, website_name);

			else
			{
				string target = components[4];
				if (target[0] == '~')
					target = link_names[target];

				links.push_back(Link(Rect(atof(components[0].c_str()), atof(components[1].c_str()), atof(components[2].c_str()), atof(components[3].c_str())), target, underline, underline_offset));
				if (parent->ran_trial && links.back().target == "download_trial")
					links.back().target = "trialexpired";

				links.back().underline_red = underline_red;
				links.back().underline_green = underline_green;
				links.back().underline_blue = underline_blue;
			}
		}
	});

	if (website_name == "rosecolored4" && !parent->visited_site[website_name])
		parent->required_target = "search";

	/*if (website_name == "ain" && parent->reached_help_center)
		links[0].target = "psych";*/

	if (website_name == "psych")
		parent->reached_psych_article = true;

	if (textures[0] == "nendablank.png")
	{
		if (parent->visited_elsewhere_before_rosecolored1 && parent->no_rosecolored1)
		{
			parent->emails.insert(parent->emails.begin(), "rosecolored1bold.png");
			parent->no_rosecolored1 = false;
			parent->whitelist.push_back("rosecolored1");
			save_game(parent->save_slot);
		}

		if (parent->visited_elsewhere_after_rosecolored1 && parent->no_rosecolored2 && parent->email_counter > 50 * 60)
		{
			parent->emails.insert(parent->emails.begin(), "rosecolored2bold.png");
			parent->no_rosecolored2 = false;
			parent->whitelist.push_back("rosecolored2");
			parent->email_counter = 0;
		}

		if (parent->looked_for_spider && parent->no_rosecolored3 && parent->email_counter > 11 * 60)
		{
			parent->emails.insert(parent->emails.begin(), "rosecolored3bold.png");
			parent->no_rosecolored3 = false;
			parent->whitelist.push_back("rosecolored3");
			parent->email_counter = 0;
		}

		if (parent->recovered_browser && parent->no_rfh3)
		{
			parent->emails.insert(parent->emails.begin(), "rfh3bold.png");
			parent->no_rfh3 = false;
			parent->whitelist.push_back("rfh3");
		}

		if (parent->tried_to_view_encrypted_folder && parent->no_rosecolored4 && parent->email_counter > 11 * 60)
		{
			parent->emails.insert(parent->emails.begin(), "rosecolored4bold.png");
			parent->whitelist.push_back("rosecolored4");
			parent->no_rosecolored4 = false;
			parent->email_counter = 0;
			save_game(parent->save_slot);
		}

		if (parent->missed_spider && parent->no_rosecolored_missed_spider && parent->email_counter > 11 * 60)
		{
			parent->emails.insert(parent->emails.begin(), "rosecoloredmissedspiderbold.png");
			parent->no_rosecolored_missed_spider = false;
			parent->whitelist.push_back("rosecoloredmissedspider");
			parent->email_counter = 0;
		}

		if (parent->reached_psychologist && parent->no_rosecolored5)
		{
			parent->emails.insert(parent->emails.begin(), "rosecolored5bold.png");
			parent->emails.insert(parent->emails.begin(), "rfh4bold.png");
			parent->no_rosecolored5 = false;
			parent->whitelist.push_back("rosecolored5");
			parent->whitelist.push_back("rfh4");
			parent->whitelist.push_back("walter1");
		}

		if (parent->entered_right_email_address && parent->no_centerracom_trial_link)
		{
			parent->emails.insert(parent->emails.begin(), "triallinkbold.png");
			parent->whitelist.push_back("triallink");
			parent->no_centerracom_trial_link = false;
		}

		if (parent->ran_trial && parent->no_centerracom_survey_link)
		{
			parent->emails.insert(parent->emails.begin(), "surveylinkbold.png");
			parent->emails.insert(parent->emails.begin(), "rfh5bold.png");
			parent->whitelist.push_back("surveylink");
			parent->whitelist.push_back("rfh5");
			parent->no_centerracom_survey_link = false;
		}

		if (parent->contest_finished && parent->no_contest_finished_email)
		{
			parent->emails.insert(parent->emails.begin(), "rosecolored6bold.png");
			parent->emails.insert(parent->emails.begin(), "rfh6bold.png");
			parent->whitelist.push_back("walter2");
			parent->whitelist.push_back("rosecolored6");
			parent->whitelist.push_back("rfh6");
			parent->no_contest_finished_email = false;
			save_game(parent->save_slot);
		}

		if (parent->beat_space_game && parent->no_rfh7)
		{
			parent->emails.insert(parent->emails.begin(), "rfh7bold.png");
			parent->whitelist.push_back("rfh7");
			parent->no_rfh7 = false;
		}

		float y1 = 1080 - 286;
		for (int i = 0; i < parent->emails.size(); ++i)
		{ //link emails
			links.push_back(Link(Rect(73.0 + 15, y1, 73 + 15 + parent->texture_manager->get_width(parent->emails[i]), y1 + parent->texture_manager->get_height(parent->emails[i])), parent->emails[i].substr(0, parent->emails[i].length() - 8)));
			y1 -= 35.0;
		}
	}
    
	add_children(website_name, skip_loading);
	parent->visited_site[website_name] = true;
	update_cursor(prev_mouse_x, prev_mouse_y);
	if (parent->focus_application == BROWSER && parent->application_open(BROWSER)) //in case someone switches to another application while site is loading
	{
		parent->queue_click_x = 0; //TODO make sure never any links at pixel 0
		parent->queue_click_y = 700;
	}
}

void ScrollPanel::tick_scrollbar(int ticks)
{
	if (parent->everything_stuck)
		return;
	
	float old_offset = offset;
    offset -= (float)(sum_of_heights * ticks) / scroll_bar->range();
    if (offset < 0.0)
        offset = 0.0;
    
    move_links(offset - old_offset);
    update_cursor(prev_mouse_x, prev_mouse_y);
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
		if (!(children[i]->no_translate))
			children[i]->translate(0, delta);
	}
}

void ScrollPanel::update_cursor(int x, int y)
{
	if (!is_mouse_over)
		return;
	
	for (int i = 0; i < links.size(); ++i)
	{
		Link link = links[i];
		if (link.rect.x1 <= x && x <= link.rect.x2 && link.rect.y1 <= y && y <= link.rect.y2 && !link.no_cursor)
		{
			parent->set_cursor("cursorhand.png");
			parent->someone_set_cursor = true;
			parent->maintain_cursor = true;
			parent->cursor_maintainer = this;
			if (link.underline)
			{
				underlinex1 = link.rect.x1;
				underlinex2 = link.rect.x2;
				underliney = link.rect.y1 - link.offset;
				underline_red = link.underline_red;
				underline_green = link.underline_green;
				underline_blue = link.underline_blue;
				parent->draw_underline = true;
			}

			else parent->draw_underline = false;

			return;
		}
	}

	parent->set_cursor("cursor.png");
	parent->maintain_cursor = false;
	parent->cursor_maintainer = NULL;
	parent->draw_underline = false;
}

void ScrollPanel::mouse_over(int x, int y)
{
	prev_mouse_x = x;
	prev_mouse_y = y;
	is_mouse_over = true;
	update_cursor(x, y);
}

void ScrollPanel::mouse_moved(int x, int y)
{
	if (!scroll_locked)
		return;

	if (y > scroll_lock_y + 10)
		scroll_lock_speed = -1 * round((y - scroll_lock_y - 10) / 5.0);

	else if (y < scroll_lock_y - 10)
		scroll_lock_speed = round((scroll_lock_y - 10 - y) / 5.0);

	else
		scroll_lock_speed = 0.0;
}

void ScrollPanel::turn_off_scroll_lock()
{
	if (scroll_locked)
	{
		scroll_locked = false;
		scroll_lock_speed = 0.0;
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
		if (!scroll_locked && prev_website != "game")
		{
			scroll_locked = true;
			scroll_lock_x = x;
			scroll_lock_y = y;
			browser_key = BrowserKey();
		}

		else
			turn_off_scroll_lock();

		return; //TODO should middle button clicking on a link take you to the site?
	}

	turn_off_scroll_lock();
	if (button == 3)
	{
		if (state == 0)
		{
			if (browser_key.key == 3)
			{
				browser_key.death_counter = 10;
				if (browser_key.frames <= 1)
				{
					browser_key.velocity += 9;
					if (browser_key.velocity > 90)
						browser_key.velocity = 90;
				}

				browser_key.frames = 0;
			}

			else browser_key = BrowserKey(3, 10);
		}

		return;
	}

	if (button == 4)
	{
		if (state == 0)
		{
			if (browser_key.key == 4)
			{
				browser_key.death_counter = 10;
				if (browser_key.frames <= 1)
				{
					browser_key.velocity += 9;
					if (browser_key.velocity > 90)
						browser_key.velocity = 90;
				}

				browser_key.frames = 0;
			}

			else browser_key = BrowserKey(4, 10);
		}

		return;
	}

	if (parent->everything_stuck && parent->breaking_stage == 7)
	{
		if (break_points.size() >= num_break_points)
		{
			if (parent->done_cracking)
			{
				if (fall_pieces < 2)
				{
					int index = glass_textures.size() - 20 - fall_pieces;
					glass_textures[index].fall();
					parent->move_range(glass_textures, index, index + 1, glass_textures.size() - 1);
					fall_pieces++;
					//parent->play_sound(5);
					return;
				}

				if (fall_pieces == 2)
				{
					for (int i = 0; i < glass_textures.size(); ++i)
					{
						glass_textures[i].fall();
						glass_textures[i].velocity_y = 5.0;
						glass_textures[i].velocity_x = rand() % 2 == 0 ? -13.0 : 13.0;
						//parent->play_sound(4);
					}
				}
			}
			
			return;
		}

		break_points.push_back(pair<int, int>(x, y));
		//parent->play_sound(4);
	}
	
	if (button != GLUT_LEFT)
		return;
	
	for (int i = 0; i < links.size(); ++i)
    {
        Link link = links[i];
		if (link.rect.x1 <= x && x <= link.rect.x2 && link.rect.y1 <= y && y <= link.rect.y2)
		{
			if (link.target.substr(0, 8) == "sequence")
			{
				if (parent->link_overrides[textures[0]][i] != "")
				{
					links[i].target = parent->link_overrides[textures[0]][i];
					link.target = parent->link_overrides[textures[0]][i];
				}

				else if (parent->sequences[link.target].empty())
				{
					if (link.target == "sequencereddit")
					{
						string new_target = "sleepy.oo.oo.zzz/thread?id=";
						string to_append = "";
						for (int i = 0; i < 5; ++i)
							to_append += random_char();

						if (num_digits(to_append) < 3)
						{
							to_append[0] = random_num();
							to_append[2] = random_num();
							to_append[4] = random_num();
						}

						new_target += to_append;
						for (auto it = website_names.begin(); it != website_names.end(); ++it)
						{
							if (it->second == new_target)
							{
								new_target[0] = 'z';
								break;
							}
						}

						links[i].target = new_target;
						link.target = new_target;
						parent->link_overrides[textures[0]][i] = new_target;
					}
				}

				else
				{
					string new_target = parent->sequences[link.target][parent->sequences[link.target].size() - 1];
					if (link.target != "sequenceain" && !(link.target == "sequenceforum" && parent->sequences[link.target].size() == 1) && new_target != "hcthread" && new_target != "decryptthread")
					{
						parent->sequences[link.target].pop_back();
						if (link.target == "sequencereddit")
							save_game(parent->save_slot);
					}

					links[i].target = new_target;
					link.target = new_target;
					parent->link_overrides[textures[0]][i] = new_target;
				}
			}

			/*if (textures[0] == "nendablank.png")
			{
				auto it = parent->read_email.find(link.target + "bold.png");
				if (it != parent->read_email.end())
					parent->read_email[link.target + "bold.png"] = true;
			}*/

			link.link_action();
			start_loading_site(link.target, true);
			will_force_through = true;
			if (link.target == "vote" || link.target == "browse" || link.target == "upload")
				loading_time = 0;
		}
    }
}

bool ScrollPanel::responds_to(int button)
{
	if (button == GLUT_LEFT)
		return true;

	else if (button == GLUT_RIGHT_BUTTON)
		return true;

	else if (button == 3)
		return true;

	else if (button == 4)
		return true;

	else if (button == 1)
		return true;
}

int ScrollPanel::find_piece(vector<vector<pair<int, int>>> pieces, pair<int, int> point)
{
	for (int i = 0; i < pieces.size(); ++i)
	{
		if (std::find(pieces[i].begin(), pieces[i].end(), point) != pieces[i].end())
			return i;
	}

	return -1;
}

void ScrollPanel::write_buffer_overlapping(GLubyte* buffer, int buffer_width, int buffer_height, GLubyte* image, int image_x, int image_y, int image_width, int image_height)
{
	int pixels_written = 0;
	for (int i = image_x; i < image_x + image_width; ++i)
	{
		for (int j = image_y; j < image_y + image_height; ++j)
		{
			if (0 <= i && i < buffer_width && 0 <= j && j < buffer_height)
			{
				int image_start = (j - image_y) * image_width * 4 + (i - image_x) * 4;
				int buffer_start = j * buffer_width * 4 + i * 4;
				float alpha = (float)(image[image_start + 3]) / 255.0;
				GLubyte r = image[image_start];
				GLubyte g = image[image_start + 1];
				GLubyte b = image[image_start + 2];
				if (r == 0 && g == 255 && b == 0)
				{
					int q = 1;
				}

				buffer[buffer_start] = buffer[buffer_start] * (1.0 - alpha) + image[image_start] * alpha;
				buffer[buffer_start + 1] = buffer[buffer_start + 1] * (1.0 - alpha) + image[image_start + 1] * alpha;
				buffer[buffer_start + 2] = buffer[buffer_start + 2] * (1.0 - alpha) + image[image_start + 2] * alpha;
				buffer[buffer_start + 3] = 0;
				pixels_written++;
			}
		}
	}

	int total = buffer_width * buffer_height;
	int q = 1;
}

vector<vector<pair<int, int>>> ScrollPanel::get_pieces(int width, int height, int xoffset, int yoffset, string glass_map_name, int big_glass_width, int big_glass_height)
{
	vector<vector<pair<int, int>>> pieces;
	GLubyte* working_image = (GLubyte*) malloc(sizeof(GLubyte) * width * height * 4);
	memset(working_image, 255, sizeof(GLubyte) * width * height * 4);
	GLubyte* glass_map = parent->texture_manager->get_pixel_data(glass_map_name);
	for (int i = 0; i < break_points.size(); ++i)
	{
		int left = break_points[i].first - xoffset - big_glass_width / 2.0;
		int right = break_points[i].first - xoffset + big_glass_width / 2.0;
		int down = break_points[i].second - yoffset - big_glass_height / 2.0;
		int up = break_points[i].second - yoffset + big_glass_height / 2.0;
		for (int m = left; m < right; ++m)
		{
			for (int n = down; n < up; ++n)
			{
				if (0 <= m && m < width && 0 <= n && n < height)
				{
					int map_start = (n - down) * big_glass_width * 4 + (m - left) * 4;
					int image_start = n * width * 4 + m * 4;
					if (glass_map[map_start + 3] != 0)
					{
						working_image[image_start] = glass_map[map_start];
						working_image[image_start + 1] = glass_map[map_start + 1];
						working_image[image_start + 2] = glass_map[map_start + 2];
						working_image[image_start + 3] = 255;
					}
				}
			}
		}
	}

	Canvas canvas = Canvas(0, 0, width, height, "", NULL, NONE);
	canvas.image = working_image;
	for (int i = 0; i < width; ++i)
	{
		for (int j = 0; j < height; ++j)
		{
			if (canvas.get_color(i, j) == Vector4(255, 255, 255, 255))
				pieces.push_back(canvas.fill_from(i, j));
		}
	}

	vector<vector<pair<int, int>>> added_points;
	for (int i = 0; i < pieces.size(); ++i)
		added_points.push_back(pieces[i]);

	bool found;
	do
	{
		found = false;
		for (int i = 0; i < pieces.size(); ++i)
		{
			auto old_points = added_points[i];
			added_points[i].clear();
			for (int j = 0; j < old_points.size(); ++j)
			{
				vector<pair<int, int>> candidates;
				candidates.push_back(pair<int, int>(old_points[j].first - 1, old_points[j].second));
				candidates.push_back(pair<int, int>(old_points[j].first + 1, old_points[j].second));
				candidates.push_back(pair<int, int>(old_points[j].first, old_points[j].second - 1));
				candidates.push_back(pair<int, int>(old_points[j].first, old_points[j].second + 1));
				for (int k = 0; k < 4; ++k)
				{
					auto color = canvas.get_color(candidates[k].first, candidates[k].second);
					if (!(color == Vector4(0, 0, 0, 255)) && !(color == Vector4(-1, -1, -1, -1)))
					{
						canvas.set_pixel(candidates[k].first, candidates[k].second, 0, 0, 0, 255);
						added_points[i].push_back(candidates[k]);
						found = true;
					}
				}
			}

			pieces[i].insert(pieces[i].end(), added_points[i].begin(), added_points[i].end());
		}
	} while (found);

	//free(working_image); TODO make sure canvas takes care of this
	return pieces;
}

//TODO using big grid glass/glassmap gives inaccurate number of glass tiles sometimes, might want to investigate
void ScrollPanel::create_glass_textures(vector<vector<pair<int, int>>> pieces, int actual_image_width, int actual_image_height, int xoffset, int yoffset, string image_name, string glass_name, int big_glass_width, int big_glass_height, string suffix)
{
	for (int i = 0; i < pieces.size(); ++i)
	{
		int minx, miny, maxx, maxy;
		minx = miny = 10000;
		maxx = maxy = -10000;
		for (int j = 0; j < pieces[i].size(); ++j)
		{
			if (pieces[i][j].first < minx)
				minx = pieces[i][j].first;

			if (pieces[i][j].first > maxx)
				maxx = pieces[i][j].first;

			if (pieces[i][j].second < miny)
				miny = pieces[i][j].second;

			if (pieces[i][j].second > maxy)
				maxy = pieces[i][j].second;
		}

		GLubyte* buffer = (GLubyte*)malloc(sizeof(GLubyte) * (maxx - minx + 1) * (maxy - miny + 1) * 4);
		memset(buffer, 0, sizeof(GLubyte) * (maxx - minx + 1) * (maxy - miny + 1) * 4);
		GLubyte* page_data = parent->texture_manager->get_pixel_data(image_name);
		write_buffer_overlapping(buffer, maxx - minx + 1, maxy - miny + 1, page_data, -minx - xoffset, -miny - yoffset, actual_image_width, actual_image_height);
		GLubyte* big_glass = parent->texture_manager->get_pixel_data(glass_name);
		for (int j = 0; j < break_points.size(); ++j)
			write_buffer_overlapping(buffer, maxx - minx + 1, maxy - miny + 1, big_glass, break_points[j].first - big_glass_width / 2.0 - minx - xoffset, break_points[j].second - big_glass_height / 2.0 - miny - yoffset, big_glass_width, big_glass_height);

		for (int j = 0; j < pieces[i].size(); ++j)
		{
			int start = (pieces[i][j].second - miny) * (maxx - minx + 1) * 4 + (pieces[i][j].first - minx) * 4;
			int image_start = pieces[i][j].second * actual_image_width * 4 + pieces[i][j].first * 4;
			buffer[start + 3] = page_data[image_start + 3];
		}
		
		parent->texture_manager->load_texture("glass" + to_string(i) + suffix, false, false, buffer, maxx - minx + 1, maxy - miny + 1);
		PhysicsPiece glass = PhysicsPiece(minx + xoffset, miny + yoffset, maxx + xoffset + 1, maxy + yoffset + 1, "glass" + to_string(i) + suffix);
		glass.invert = true;
		glass_textures.push_back(glass);
		free(buffer);
	}

	std::sort(glass_textures.begin(), glass_textures.end(), [](PhysicsPiece a, PhysicsPiece b) {return (a.y2 - a.y1) * (a.x2 - a.x1) < (b.y2 - b.y1) * (b.x2 - b.x1);});
}

void ScrollPanel::never_focus_children()
{
	for (int i = 0; i < children.size(); ++i)
		children[i]->never_focus = true;
}

void ScrollPanel::ask_for_delete(ScreenElement* elem)
{
	elem->marked_for_deletion = true;
	for (int i = 0; i < children.size(); ++i)
	{
		if (children[i] == elem)
		{
			children.erase(children.begin() + i);
			break;
		}
	}
}

void ScrollPanel::mouse_off()
{
	//I don't remember why I had to do this
	//And I don't remember why I decided to comment it out either
    //glutSetCursor(GLUT_CURSOR_LEFT_ARROW);

	is_mouse_over = false;
	if (parent->maintain_cursor && parent->cursor_maintainer == this)
		parent->maintain_cursor = false;

	parent->draw_underline = false;
}

void ScrollPanel::go_home()
{
	start_loading_site("search");
}

void ScrollPanel::go_back()
{
    if (history_index == 0)
        return;
    
    history_index--;
    change_website(history[history_index].first, false, true, true);
	if (history[history_index].first == "error")
	{
		text_field_ptr->reset();
		text_field_ptr->text = history[history_index].second;
	}
}

void ScrollPanel::go_forward()
{
    if (history_index < (history.size() - 1))
    {
        history_index++;
        change_website(history[history_index].first, false, true, true);
		if (history[history_index].first == "error")
		{
			text_field_ptr->reset();
			text_field_ptr->text = history[history_index].second;
		}
    }
}